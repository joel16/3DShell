#include "common.h"
#include "file/dirlist.h"
#include "menus/status_bar.h"
#include "screenshot.h"
#include "text.h"
#include "utils.h"

int displayText(char * path)
{
	//sf2d_texture * textView = sfil_load_PNG_file("romfs:/res/textview.png", SF2D_PLACE_RAM); setBilinearFilter(textView);

	//sf2d_set_clear_color(RGBA8(251, 251, 251, 255));

	FILE * file = fopen(path, "rb");

	char line[BUFSIZ];

	while (fgets(line, sizeof(line), file))
	{
		puts(line);
	}

	fclose(file);

	while (aptMainLoop())
	{
		hidScanInput();

		//sf2d_start_frame(GFX_BOTTOM, GFX_LEFT); // Clear bottom screen
		//sf2d_end_frame();

		//sf2d_start_frame(GFX_TOP, GFX_LEFT);

		//sf2d_draw_texture(textView, 0, 0);

		drawWifiStatus();
		drawBatteryStatus();
		digitalTime();

		//sftd_draw_textf(font, 40, 27, RGBA8(251, 251, 251, 255), 11, "%s", fileName);
		//sftd_draw_textf(font, 10, 58, RGBA8(0, 0, 0, 255), 11, "%s", line);
		//draw_ui(path, current_line, &fv);

		/*if (kPressed & KEY_DUP)
		{
			if (current_line > 0)
			{
				current_line -= 1;
				draw_ui(path, current_line, &fv);
			}
		}

		else if (kPressed & KEY_DDOWN)
		{
			if (current_line+30 <= fv.lines-1)
			{
				current_line += 1;
				draw_ui(path, current_line, &fv);
			}
		}*/

		//endDrawing();

		if (kPressed & KEY_B)
		{
			wait(1);
			break;
		}

		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}

	//sf2d_free_texture(textView);
	return 0;
}
