#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archive.h"
#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static char *Archive_RemoveFileExt(char *filename) {
	char *ret, *lastdot;

   	if (filename == NULL)
   		return NULL;
   	if ((ret = malloc(strlen(filename) + 1)) == NULL)
   		return NULL;

   	strcpy(ret, filename);
   	lastdot = strrchr(ret, '.');

   	if (lastdot != NULL)
   		*lastdot = '\0';

   	return ret;
}

static u64 Archive_CountFiles(const char *path) {
	int ret = 0;
	u64 count = 0;

    struct archive *handle = archive_read_new();

	ret = archive_read_support_format_all(handle);

    ret = archive_read_open_filename(handle, path, 1024);
    if (ret != ARCHIVE_OK) {
		Menu_DisplayError("archive_read_open_filename :", ret);
        return 1;
    }

	for (;;) {
		struct archive_entry *entry = NULL;
        int ret = archive_read_next_header(handle, &entry);
        if (ret == ARCHIVE_EOF)
            break;
		
		count++;
	}

	ret = archive_read_free(handle);
	return count;
}

static int Archive_WriteData(struct archive *src, struct archive *dst) {
	int ret = 0;
	
	for (;;) {
		const void *chunk = NULL;
		size_t length = 0;
		s64 offset = 0;
		
		ret = archive_read_data_block(src, &chunk, &length, &offset);
		if (ret == ARCHIVE_EOF)
			return ARCHIVE_OK;
		
		if (ret != ARCHIVE_OK)
			return ret;
			
		ret = archive_write_data_block(dst, chunk, length, offset);
		if (ret != ARCHIVE_OK)
			return ret;
	}
	
	return -1;
}

int Archive_ExtractArchive(const char *path) {
	char *dest_path = malloc(256);
	char *dirname_without_ext = Archive_RemoveFileExt((char *)path);

	snprintf(dest_path, 512, "%s/", dirname_without_ext);
	FS_MakeDir(archive, dest_path);

	int count = 0;
	u64 max = Archive_CountFiles(path);

    int ret = 0;
    struct archive *handle = archive_read_new();
    struct archive *dst = archive_write_disk_new();

	ret = archive_read_support_format_all(handle);

    ret = archive_read_open_filename(handle, path, 1024);
    if (ret != ARCHIVE_OK) {
		Menu_DisplayError("archive_read_open_filename :", ret);
        return 1;
    }

    for (;;) {
		ProgressBar_DisplayProgress("Extracting", path, count, max);

        struct archive_entry *entry = NULL;
        ret = archive_read_next_header(handle, &entry);

        if (ret == ARCHIVE_EOF)
            break;

        if (ret != ARCHIVE_OK) {
			Menu_DisplayError("archive_read_next_header failed:", ret);

            if (ret != ARCHIVE_WARN)
                break;
        }

        const char *entry_path = archive_entry_pathname(entry);

        char new_path[1024 * 4];
        ret = snprintf(new_path, sizeof(new_path), "%s/%s", dest_path, entry_path);

        ret = archive_entry_update_pathname_utf8(entry, new_path);
        if (!ret) {
			Menu_DisplayError("archive_entry_update_pathname_utf8:", ret);
            break;
        }

		ret = archive_write_disk_set_options(dst, ARCHIVE_EXTRACT_UNLINK);
        ret = archive_write_header(dst, entry);
        if (ret != ARCHIVE_OK) {
			Menu_DisplayError("archive_write_header failed:", ret);
            break;
        }

        ret = Archive_WriteData(handle, dst);
        ret = archive_write_finish_entry(dst);
		count++;
    }

    ret = archive_write_free(dst);
    ret = archive_read_free(handle);
	free(dest_path);
	free(dirname_without_ext);
    return ret;
}

Result Archive_ExtractFile(const char *path) {
	int dialog_selection = 0;
	float text_width1 = 0, text_width2 = 0, confirm_width = 0, confirm_height = 0, cancel_width = 0, cancel_height = 0;
	
	Draw_GetTextSize(0.45f, &text_width1, NULL, "This may take a few minutes.");
	Draw_GetTextSize(0.45f, &text_width2, NULL, "Do you want to continue?");
	Draw_GetTextSize(0.45f, &confirm_width, &confirm_height, "YES");
	Draw_GetTextSize(0.45f, &cancel_width, &cancel_height, "NO");

	char extension[5] = {0};
	strncpy(extension, &path[strlen(path) - 4], 4);

	while(aptMainLoop()) {
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar

		Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

		Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Extract file");

		Draw_Text(((320 - (text_width1)) / 2), ((240 - (dialog.subtex->height)) / 2) + 35, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "This may take a few minutes.");
		Draw_Text(((320 - (text_width2)) / 2), ((240 - (dialog.subtex->height)) / 2) + 50, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Do you wish to continue?");

		if (dialog_selection == 0)
			Draw_Rect((288 - cancel_width) - 5, (159 - cancel_height) - 5, cancel_width + 10, cancel_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
		else if (dialog_selection == 1)
			Draw_Rect((248 - (confirm_width)) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

		Draw_Text(248 - (confirm_width), (159 - confirm_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
		Draw_Text(288 - cancel_width, (159 - cancel_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");

		Draw_EndFrame();

		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_RIGHT)
			dialog_selection++;
		else if (kDown & KEY_LEFT)
			dialog_selection--;

		Utils_SetMax(&dialog_selection, 0, 1);
		Utils_SetMin(&dialog_selection, 1, 0);

		if (kDown & KEY_B)
			break;

		if (kDown & KEY_A) {
			if (dialog_selection == 1)
				return Archive_ExtractArchive(path);
			else
				break;
		}

		if (TouchInRect((288 - cancel_width) - 5, (159 - cancel_height) - 5, ((288 - cancel_width) - 5) + cancel_width + 10, ((159 - cancel_height) - 5) + cancel_height + 10)) {
			dialog_selection = 0;

			if (kDown & KEY_TOUCH)
				break;
		}
		else if (TouchInRect((248 - (confirm_width)) - 5, (159 - confirm_height) - 5, ((248 - (confirm_width)) - 5) + confirm_width + 10, ((159 - confirm_height) - 5) + confirm_height + 10)) {
			dialog_selection = 1;

			if (kDown & KEY_TOUCH)
				return Archive_ExtractArchive(path);
		}
	}

	return -1;
}
