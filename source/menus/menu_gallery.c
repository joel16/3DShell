#include <math.h>
#include <string.h>

#include "common.h"
#include "file/dirlist.h"
#include "graphics/screen.h"
#include "menus/menu_gallery.h"
#include "screenshot.h"
#include "utils.h"

void menu_displayImage(char * path)
{
	/*float scale = 1.0f, zoom_factor = 0.025f;
	float zoom_min = 0.25f, zoom_max = 4.0f;
	float rad = 0;

	int top_tex_x = 0, top_tex_y = 0;
	int bottom_tex_x = 40, bottom_tex_y = 240;
	int view_factor = 4;*/

	screen_clear(GFX_TOP, RGBA8(33, 39, 43, 255));
	screen_clear(GFX_BOTTOM, RGBA8(33, 39, 43, 255));

	File * file = getFileIndex(position);

	if (strncasecmp(file->ext, "png", 3) == 0)
		screen_load_texture_png(TEXTURE_GALLERY_IMAGE, path, true);
	else if (strncasecmp(file->ext, "gif", 3) == 0)
		screen_load_texture_gif(TEXTURE_GALLERY_IMAGE, path, true);
	else if (strncasecmp(file->ext, "jpg", 3) == 0)
		screen_load_texture_jpg(TEXTURE_GALLERY_IMAGE, path, true);
	else if (strncasecmp(file->ext, "bmp", 3) == 0)
		screen_load_texture_bmp(TEXTURE_GALLERY_IMAGE, path, true);

	int galleryBarY = 0, galleryBarLimY = -35, nameY = 11, nameLimY = -11;

	u64 start = osGetTime();

	bool bothScreens = false;

	u32 width = screen_get_texture_width(TEXTURE_GALLERY_IMAGE);
	u32 height = screen_get_texture_height(TEXTURE_GALLERY_IMAGE);

	while (aptMainLoop())
	{
		hidScanInput();
		hidTouchRead(&touch);

		screen_begin_frame();
		screen_select(GFX_BOTTOM);

		if (bothScreens == true)
			screen_draw_texture_crop(TEXTURE_GALLERY_IMAGE, -40, 0, 360, (height / 2));

		screen_select(GFX_TOP);

		if ((width <= 400) && (height <= 240))
			screen_draw_texture(TEXTURE_GALLERY_IMAGE, ((400 - width) / 2), ((240 - height) / 2));
		else if ((width >= 400) && (height >= 480)) // Both screens
		{
			screen_draw_texture(TEXTURE_GALLERY_IMAGE, ((400 - width) / 2), 0);
			bothScreens = true;
		}

		if (osGetTime() - start >= (2000))
		{
			nameY -= 2;
			galleryBarY -= 2;
		}

		if (galleryBarY == -35)
			galleryBarY = galleryBarLimY;
		if (nameY == -11)
			nameY = nameLimY;

		screen_draw_texture(TEXTURE_GALLERY_BAR, 0, galleryBarY);
		screen_draw_stringf(30, nameY, 0.45f, 0.45f, RGBA8(255, 255, 255, 255), "%.60s", fileName);

		if (kPressed & KEY_TOUCH)
		{
			galleryBarY = 0;
			nameY = 11;
			start = osGetTime();
		}

		/*if (kHeld & KEY_DUP) // Zoom in
		{
			wait(5);
			if (scale < zoom_max)
				scale += zoom_factor;
		}
		else if (kHeld & KEY_DDOWN) // Zoom out
		{
			wait(5);
			if (scale > zoom_min) // Don't zoom out any further than a 1/4 of the image.
				scale -= zoom_factor;
		}

		if (kPressed & KEY_L) // Rotate anti clockwise
		{
			rad -= M_PI_2;
			if (rad < 0)
				rad += M_TWOPI;
		}
		else if (kPressed & KEY_R) // Rotate clockwise
		{
			rad += M_PI_2;
			if (rad >= M_TWOPI)
				rad -= M_TWOPI;
		}

		if ((scale > 1.0f) && (kHeld & KEY_CPAD_UP))
		{
			top_tex_y -= view_factor;
			bottom_tex_y -= view_factor;
		}
		else if ((scale > 1.0f) && (kHeld & KEY_CPAD_DOWN))
		{
			top_tex_y += view_factor;
			bottom_tex_y += view_factor;
		}
		else if ((scale > 1.0f) && (kHeld & KEY_CPAD_RIGHT))
		{
			top_tex_x += view_factor;
			bottom_tex_x += view_factor;
		}
		else if ((scale > 1.0f) && (kHeld & KEY_CPAD_LEFT))
		{
			top_tex_x -= view_factor;
			bottom_tex_x -= view_factor;
		}*/

		screen_end_frame();

		if (kPressed & KEY_B)
		{
			/*if ((rad != 0) || (scale != 1.0f)) // Basically reset the position before exiting
			{
				rad = 0;
				scale = 1.0f;
				top_tex_x = 0;
				top_tex_y = 0;
				bottom_tex_x = 40;
				bottom_tex_y = 240;
			}
			else
			{*/
				wait(10);
				break;
			//}
		}

		if (((kHeld & KEY_L) && (kPressed & KEY_R)) || ((kHeld & KEY_R) && (kPressed & KEY_L)))
			captureScreenshot();
	}

	// delete image
	screen_unload_texture(TEXTURE_GALLERY_IMAGE);
}
