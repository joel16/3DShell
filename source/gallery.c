#include "bmp.h"
#include "common.h"
#include "dirlist.h"
#include "gallery.h"
#include "screenshot.h"
#include "utils.h"

void displayImage(char * path, int ext)
{
	float scale = 1.0f;
	float zoom_min = 0.25f;
	float zoom_max = 2.0f;
	float zoom_factor = 0.025f;
	float rad = 0;
	
	sf2d_texture * image = NULL;
	
	switch (ext)
	{
		case 0:
			image = sfil_load_PNG_file(path, SF2D_PLACE_RAM);
			break;
		
		case 1:
			image = sfil_load_JPEG_file(path, SF2D_PLACE_RAM);
			break;
		
		case 2: // Supported formats: GIF, HDR, PIC, PNM, PSD, TGA.
			image = sfil_load_IMG_file(path, SF2D_PLACE_RAM);
			break;
		
		case 3: // Becasue sfil_load_BMP_file is broken.
			image = sfil_load_BMP_file2(path);
			break;
	}
	
	setBilinearFilter(image);
	
	sf2d_set_clear_color(RGBA8(33, 39, 43, 255));
	
	int galleryBarY = 0, galleryBarLimY = -35, nameY = 11, nameLimY = -11;
	
	uint64_t start = osGetTime();
	
	bool bothScreens = false;
		
	while (aptMainLoop())
	{
		hidScanInput();
		hidTouchRead(&touch);
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT); // Clear bottom screen
		
		if (bothScreens == true)
			sf2d_draw_texture_part_rotate_scale(image, 0 + 160, 0 + 120, rad, 40, 240, 320, 240, scale, scale);
		
		sf2d_end_frame();
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		
		if ((image->width <= 400) && (image->height <= 240))
			sf2d_draw_texture_part_rotate_scale(image, 0 + 200, 0 + 120, rad, 0, 0, image->width, image->height, scale, scale);
		else if ((image->width == 400) && ((image->height == 480) || (image->height == 482))) // Both screens
		{
			sf2d_draw_texture_part_rotate_scale(image, 0 + 200, 0 + 120, rad, 0, 0, 400, 240, scale, scale);
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
			
		sf2d_draw_texture(galleryBar, 0, galleryBarY);
		sftd_draw_textf(font, 30, nameY, RGBA8(255, 255, 255, 255), 11, "%.60s", fileName);
		
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
		
		endDrawing();
		
		if (kPressed & KEY_B)
		{
			wait(100000000);
			break;
		}
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}
	
	//delete image
	sf2d_free_texture(image);
}