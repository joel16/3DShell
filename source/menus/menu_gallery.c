#include <stdlib.h>
#include <3ds.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "fs.h"

#include "touch.h"
#include "utils.h"

enum IMAGE_STATES {
	DIMENSION_DEFAULT = 0,
	DIMENSION_NINTENDO_SCREENSHOT = 1,
	DIMENSION_3DSHELL_SCREENSHOT = 2
};

static char album[512][512];
static int count = 0, selection = 0, dimensions = 0, pos_x = 0, pos_y = 0;
static float width = 0, height = 0;
static float scale_factor = 0.0f;
static C2D_Image image;

static void Gallery_FreeImage(C2D_Image *image) {
	C3D_TexDelete(image->tex);
	linearFree((Tex3DS_SubTexture *)image->subtex);
	C2D_TargetClear(RENDER_TOP, C2D_Color32(33, 39, 43, 255));
	C2D_TargetClear(RENDER_BOTTOM, C2D_Color32(33, 39, 43, 255));
}

static bool Gallery_DrawImage(C2D_Image image, float x, float y, float start, float end, float w, float h, float zoom_factor) {
	C2D_DrawParams params = {
		{ x - (pos_x * zoom_factor - pos_x) / 2, y - (pos_y * zoom_factor - pos_y) / 2, w * zoom_factor, h * zoom_factor },
		{ start, end },
		0.0f, 0.0f
	};

	return C2D_DrawImage(image, &params, NULL);
}

static Result Gallery_GetImageList(void) {
	Handle dir;
	Result ret = 0;

	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_ASCII, cwd)))) {
		u32 entryCount = 0;
		FS_DirectoryEntry *entries = (FS_DirectoryEntry *)calloc(MAX_FILES, sizeof(FS_DirectoryEntry));

		if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries))) {
			qsort(entries, entryCount, sizeof(FS_DirectoryEntry), Utils_Alphasort);
			char name[256] = {'\0'};

			for (u32 i = 0; i < entryCount; i++) {
				Utils_U16_To_U8((u8 *)&name[0], entries[i].name, 255);

				if ((!strncasecmp(entries[i].shortExt, "bmp", 3)) || (!strncasecmp(entries[i].shortExt, "gif", 3))
					|| (!strncasecmp(entries[i].shortExt, "jpg", 3)) || (!strncasecmp(entries[i].shortExt, "jpe", 3))
					|| (!strncasecmp(entries[i].shortExt, "pcx", 3)) || (!strncasecmp(entries[i].shortExt, "png", 3))
					|| (!strncasecmp(entries[i].shortExt, "pgm", 3)) || (!strncasecmp(entries[i].shortExt, "ppm", 3))
					|| (!strncasecmp(entries[i].shortExt, "tga", 3))) {
					strcpy(album[count], cwd);
					strcpy(album[count] + strlen(album[count]), name);
					count++;
				}
			}
		}
		else {
			free(entries);
			return ret;
		}

		free(entries);

		if (R_FAILED(ret = FSDIR_Close(dir))) // Close directory
			return ret;
	}
	else
		return ret;

	return 0;
}

static int Gallery_GetCurrentIndex(char *path) {
	for(int i = 0; i < count; ++i) {
		if (!strcmp(album[i], path))
			return i;
	}

	return 0;
}

static void Gallery_LoadTexture(char *path) {
	selection = Gallery_GetCurrentIndex(path);

	char extension[5] = {0};
	strncpy(extension, &path[strlen(path) - 4], 4);

	if (!strncasecmp(extension, ".gif", 4))
		Draw_LoadImageFileGIF(&image, path);
	else if (!strncasecmp(extension, ".pcx", 4))
		Draw_LoadImageFilePCX(&image, path);
	else
		Draw_LoadImageFile(&image, path);

	if ((image.subtex->width == 432) && (image.subtex->height == 528)) // Nintnedo's screenshot (both screens) dimensions.
		dimensions = DIMENSION_NINTENDO_SCREENSHOT;
	else if ((image.subtex->width == 400) && ((image.subtex->height == 480) || (image.subtex->height == 482)))
		dimensions = DIMENSION_3DSHELL_SCREENSHOT;
	else {
		dimensions = DIMENSION_DEFAULT;

		if ((float)image.subtex->height > 240.0f) {
			scale_factor = (240.0f / (float)image.subtex->height);
			width = (float)image.subtex->width * scale_factor;
			height = (float)image.subtex->height * scale_factor;
		}
		else {
			width = (float)image.subtex->width;
			height = (float)image.subtex->height;
		}
	}
}

static void Gallery_HandleNext(bool forward) {
	if (forward)
		selection++;
	else
		selection--;

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	Gallery_FreeImage(&image);
	Gallery_LoadTexture(album[selection]);
}

void Gallery_DisplayImage(char *path) {
	Gallery_GetImageList();
	Gallery_LoadTexture(path);

	u64 last_time = osGetTime(), current_time = 0;

	float zoom_factor = 1.0f;
	pos_x = 0;
	pos_y = 0;

	while(aptMainLoop()) {
		current_time = osGetTime();
		u64 delta_time = current_time - last_time;
		last_time = current_time;

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, C2D_Color32(33, 39, 43, 255));
		C2D_TargetClear(RENDER_BOTTOM, C2D_Color32(33, 39, 43, 255));
		C2D_SceneBegin(RENDER_TOP);

		switch (dimensions) {
			case DIMENSION_DEFAULT:
				Gallery_DrawImage(image, ((400 - (width * zoom_factor)) / 2), ((240 - (height * zoom_factor)) / 2), 0, 0, 
					width, height, zoom_factor);
				break;

			case DIMENSION_NINTENDO_SCREENSHOT:
				Gallery_DrawImage(image, 0, 0, 16, 16, image.subtex->width, image.subtex->height, 1.0f);
				break;

			case DIMENSION_3DSHELL_SCREENSHOT:
				Gallery_DrawImage(image, 0, 0, 0, 0, image.subtex->width, image.subtex->height, 1.0f);
				break;

			default:
				break;
		}

		C2D_SceneBegin(RENDER_BOTTOM);

		switch (dimensions) {
			case DIMENSION_NINTENDO_SCREENSHOT:
				Gallery_DrawImage(image, 0, 0, 56, 272, image.subtex->width, image.subtex->height, 1.0f);
				break;

			case DIMENSION_3DSHELL_SCREENSHOT:
				Gallery_DrawImage(image, 0, 0, 40, 240, image.subtex->width, image.subtex->height, 1.0f);
				break;

			default:
				break;
		}

		hidScanInput();
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		if ((kDown & KEY_DLEFT) || (kDown & KEY_L))
			Gallery_HandleNext(false);
		else if ((kDown & KEY_DRIGHT) || (kDown & KEY_R))
			Gallery_HandleNext(true);

		if (dimensions == DIMENSION_DEFAULT) {
			if ((height * zoom_factor > 240) || (width * zoom_factor > 400)) {
				double velocity = 2 / zoom_factor;

				if (kHeld & KEY_CPAD_UP)
					pos_y -= ((velocity * zoom_factor) * delta_time);
				else if (kHeld & KEY_CPAD_DOWN)
					pos_y += ((velocity * zoom_factor) * delta_time);
				else if (kHeld & KEY_CPAD_LEFT)
					pos_x -= ((velocity * zoom_factor) * delta_time);
				else if (kHeld & KEY_CPAD_RIGHT)
					pos_x += ((velocity * zoom_factor) * delta_time);
			}

			if ((kHeld & KEY_DUP) || (kHeld & KEY_CSTICK_UP)) {
				zoom_factor += 0.5f * (delta_time * 0.001);

				if (zoom_factor > 2.0f)
					zoom_factor = 2.0f;
			}
			else if ((kHeld & KEY_DDOWN) || (kHeld & KEY_CSTICK_DOWN)) {
				zoom_factor -= 0.5f * (delta_time * 0.001);

				if (zoom_factor < 0.5f)
					zoom_factor = 0.5f;

				if (zoom_factor <= 1.0f) {
					pos_x = 0;
					pos_y = 0;
				}
			}

			if (kDown & KEY_SELECT) { // Reset zoom/pos
				pos_x = 0;
				pos_y = 0;
				zoom_factor = 1.0f;
			}
		}

		Utils_SetMax(&pos_x, width, width);
		Utils_SetMin(&pos_x, -width, -width);
		Utils_SetMax(&pos_y, height, height);
		Utils_SetMin(&pos_y, -height, -height);

		Draw_EndFrame();

		if (kDown & KEY_B)
			break;
	}

	Gallery_FreeImage(&image);
	memset(album, 0, sizeof(album[0][0]) * 512 * 512);
	count = 0;
	MENU_STATE = MENU_STATE_HOME;
}
