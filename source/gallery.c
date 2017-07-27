#include "common.h"
#include "dirlist.h"
#include "gallery.h"
#include "screen.h"
#include "screenshot.h"
#include "utils.h"

void displayImage(char * path)
{
	float scale = 1.0f, zoom_factor = 0.025f;
	float zoom_min = 0.25f, zoom_max = 4.0f;
	float rad = 0;
	
	int top_tex_x = 0, top_tex_y = 0;
	int bottom_tex_x = 40, bottom_tex_y = 240;
	int view_factor = 4;
	
	screen_load_texture_file(TEXTURE_GALLERY_IMAGE, path, true);
	
	//sf2d_set_clear_color(RGBA8(33, 39, 43, 255));
	
	int galleryBarY = 0, galleryBarLimY = -35, nameY = 11, nameLimY = -11;
	
	u64 start = osGetTime();
	
	//bool bothScreens = false;
		
	while (aptMainLoop())
	{
		hidScanInput();
		hidTouchRead(&touch);
		
		screen_begin_frame();
		
		screen_select(GFX_BOTTOM);
		
		/*if (bothScreens == true)
			sf2d_draw_texture_part_rotate_scale(image, 0 + 160, 0 + 120, rad, bottom_tex_x, bottom_tex_y, 320, 240, scale, scale);
		
		screen_select(GFX_TOP);
		
		if ((image->width <= 400) && (image->height <= 240))
			sf2d_draw_texture_part_rotate_scale(image, 0 + 200, 0 + 120, rad, 0, 0, image->width, image->height, scale, scale);
		else if ((image->width == 400) && ((image->height == 480) || (image->height >= 480 && image->height <= 482))) // Both screens
		{
			sf2d_draw_texture_part_rotate_scale(image, 0 + 200, 0 + 120, rad, top_tex_x, top_tex_y, 400, 240, scale, scale);
			bothScreens = true;
		}*/
		
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
		screen_draw_stringf(30, nameY, 0.41f, 0.41f, RGBA8(255, 255, 255, 255), "%.60s", fileName);
		
		if (kPressed & KEY_TOUCH)
		{
			galleryBarY = 0;
			nameY = 11;
			start = osGetTime();
		}
		
		if (kHeld & KEY_DUP) // Zoom in
		{
			wait(5000000);
			if (scale < zoom_max)
				scale += zoom_factor;
		}
		else if (kHeld & KEY_DDOWN) // Zoom out
		{
			wait(5000000);
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
		}
		
		screen_end_frame();
		
		if (kPressed & KEY_B)
		{
			if ((rad != 0) || (scale != 1.0f)) // Basically reset the position before exiting
			{
				rad = 0;
				scale = 1.0f;
				top_tex_x = 0;
				top_tex_y = 0;
				bottom_tex_x = 40;
				bottom_tex_y = 240;
			}
			else
			{
				wait(100000000);
				break;
			}
		}
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}
	
	//delete image
	screen_unload_texture(TEXTURE_GALLERY_IMAGE);
}