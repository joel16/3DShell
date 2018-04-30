#include <math.h>
#include <string.h>

#include "common.h"
#include "dir_list.h"
#include "menu_gallery.h"
#include "pp2d.h"
#include "screenshot.h"
#include "textures.h"
#include "utils.h"

#define DIMENSION_DEFAULT             0
#define DIMENSION_NINTENDO_SCREENSHOT 1
#define DIMENSION_NINTENDO_PICTURE    2
#define DIMENSION_3DSHELL_SCREENSHOT  3
#define DIMENSION_OTHER               4

#define GALLERY_BAR_Y_BOUNDARY  -35
#define GALLERY_NAME_Y_BOUNDARY -11

void Gallery_DisplayImage(char * path)
{
	File * file = Dirlist_GetFileIndex(position);

	if (strncasecmp(file->ext, "png", 3) == 0)
		pp2d_load_texture_png(TEXTURE_GALLERY_IMAGE, path);
	else if (strncasecmp(file->ext, "bmp", 3) == 0)
		pp2d_load_texture_bmp(TEXTURE_GALLERY_IMAGE, path);
	else if (strncasecmp(file->ext, "jpg", 3) == 0)
		pp2d_load_texture_jpg(TEXTURE_GALLERY_IMAGE, path);
	else if ((strncasecmp(file->ext, "gif", 3) == 0) || (strncasecmp(file->ext, "tga", 3) == 0))
		pp2d_load_texture_other(TEXTURE_GALLERY_IMAGE, path);

	int galleryBarY = 0, nameY = 11;

	u64 start = osGetTime();

	float width = pp2d_get_texture_width(TEXTURE_GALLERY_IMAGE);
	float height = pp2d_get_texture_height(TEXTURE_GALLERY_IMAGE);

	int dimensions = 0;

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		pp2d_begin_draw(GFX_BOTTOM, GFX_LEFT);

			pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(33, 39, 43, 255));

			switch (dimensions)
			{
				case DIMENSION_NINTENDO_SCREENSHOT:
					pp2d_draw_texture_part(TEXTURE_GALLERY_IMAGE, 0, 0, 56, 272, 320, 240);
					break;

				case DIMENSION_3DSHELL_SCREENSHOT:
					pp2d_draw_texture_part(TEXTURE_GALLERY_IMAGE, 0, 0, 40, 240, 320, 240);
					break;
			}
		
		pp2d_end_draw();

		pp2d_begin_draw(GFX_TOP, GFX_LEFT);

			pp2d_draw_rectangle(0, 0, 400, 240, RGBA8(33, 39, 43, 255));

			if ((width <= 400.0) && (height <= 240.0))
				DIMENSION_DEFAULT;
			else if ((width == 432) && (height == 528)) // Nintnedo's screenshot (both screens) dimensions.
				dimensions = DIMENSION_NINTENDO_SCREENSHOT;
			else if ((width == 640) && (height == 480)) // Nintnedo's CAM dimensions.
				dimensions = DIMENSION_NINTENDO_PICTURE;
			else if ((width == 400) && (height == 480))
				dimensions = DIMENSION_3DSHELL_SCREENSHOT;
			else if ((width > 400.0) && (height > 240.0))
				dimensions = DIMENSION_OTHER;

			switch (dimensions)
			{
				case DIMENSION_DEFAULT:
					pp2d_draw_texture(TEXTURE_GALLERY_IMAGE, ((400.0 - width) / 2.0), ((240.0 - height) / 2.0));
					break;

				case DIMENSION_NINTENDO_SCREENSHOT:
					pp2d_draw_texture_part(TEXTURE_GALLERY_IMAGE, 0, 0, 16, 16, 400, 240);
					break;

				case DIMENSION_NINTENDO_PICTURE:
					pp2d_draw_texture_scale(TEXTURE_GALLERY_IMAGE, 40, 0, 0.5, 0.5);
					break;

				case DIMENSION_3DSHELL_SCREENSHOT:
					pp2d_draw_texture_part(TEXTURE_GALLERY_IMAGE, 0, 0, 0, 0, 400, 240);
					break;

				case DIMENSION_OTHER:
					pp2d_draw_texture_scale(TEXTURE_GALLERY_IMAGE, 0, 0, 400.0/width, 240.0/height);
					break;
			}

			if (galleryDisplay)
			{
				if (osGetTime() - start >= (1500))
				{
					nameY -= 4;
					galleryBarY -= 4;
				}

				if (galleryBarY == -35)
					galleryBarY = GALLERY_BAR_Y_BOUNDARY;
				if (nameY == -11)
					nameY = GALLERY_NAME_Y_BOUNDARY;

				pp2d_draw_texture(TEXTURE_GALLERY_BAR, 0, galleryBarY);
				pp2d_draw_textf(30, nameY, 0.45f, 0.45f, RGBA8(255, 255, 255, 255), "%.60s", fileName);

				if (kDown & KEY_TOUCH)
				{
					galleryBarY = 0;
					nameY = 11;
					start = osGetTime();
				}
			}

		pp2d_end_draw();

		if (kDown & KEY_B)
		{
			wait(10);
			break;
		}

		if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
			Screenshot_Capture();
	}

	pp2d_free_texture(TEXTURE_GALLERY_IMAGE); // delete image
}
