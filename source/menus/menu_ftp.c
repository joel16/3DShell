#include <stdio.h>
#include <string.h>

#include "common.h"
#include "file/dirlist.h"
#include "file/fs.h"
#include "graphics/screen.h"
#include "language.h"
#include "menus/menu_ftp.h"
#include "net/ftp.h"
#include "screenshot.h"
#include "task.h"
#include "theme.h"
#include "utils.h"

struct colour Storage_colour;
struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;

void menu_displayFTP()
{
	ftp_init();
	task_init();

	touchPosition touch;
	
	char buf[25];
	u32 wifiStatus = 0;

	int pBar = 0, xlim = 270;

	while(aptMainLoop())
	{
		screen_begin_frame();
		screen_select(GFX_BOTTOM);

		hidScanInput();
		hidTouchRead(&touch);

		if (((kPressed & KEY_TOUCH) && (touchInRect(98, 123, 0, 20))) || (kPressed & KEY_SELECT))
			break;

		screen_draw_rect(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
		screen_draw_rect(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));

		screen_draw_texture(TEXTURE_HOME_ICON, -2, -2);
		screen_draw_texture(TEXTURE_OPTIONS_ICON, 25, 0);
		screen_draw_texture(TEXTURE_SETTINGS_ICON, 50, 1);
		screen_draw_texture(TEXTURE_UPDATE_ICON, 75, 0);
		screen_draw_texture(TEXTURE_FTP_ICON_SELECTED, 100, 0);

		if (BROWSE_STATE == STATE_SD)
			screen_draw_texture(TEXTURE_SD_ICON_SELECTED, (320 - screen_get_texture_width(TEXTURE_SD_ICON_SELECTED)) - 55, 0);
		else
			screen_draw_texture(TEXTURE_SD_ICON, (320 - screen_get_texture_width(TEXTURE_SD_ICON)) - 55, 0);

		if (BROWSE_STATE == STATE_NAND)
			screen_draw_texture(TEXTURE_NAND_ICON_SELECTED, (320 - screen_get_texture_width(TEXTURE_NAND_ICON_SELECTED)) - 30, 0);
		else
			screen_draw_texture(TEXTURE_NAND_ICON, (320 - screen_get_texture_width(TEXTURE_NAND_ICON)) - 30, 0);

		screen_draw_texture(TEXTURE_SEARCH_ICON, (320 - screen_get_texture_width(TEXTURE_SEARCH_ICON)), -2);

		ftp_loop();

		ACU_GetWifiStatus(&wifiStatus);

		if (!(wifiStatus))
		{
			screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][3], 0.45f, 0.45f)) / 2), 40, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][3]);
			sprintf(buf, lang_ftp[language][4]);
		}
		else
		{
			screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][0], 0.45f, 0.45f)) / 2), 40, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][0]);

			u32 ip = gethostid();
			sprintf(buf, "IP: %lu.%lu.%lu.%lu:5000", ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF);

			screen_draw_string(((320 - screen_get_string_width(buf, 0.45f, 0.45f)) / 2), 60, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), buf);

			if (strlen(ftp_accepted_connection) != 0)
				screen_draw_string(((320 - screen_get_string_width(ftp_accepted_connection, 0.45f, 0.45f)) / 2), 80, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), ftp_accepted_connection);

			screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][1], 0.45f, 0.45f)) / 2), 100, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][1]);

			if (strlen(ftp_file_transfer) != 0)
				screen_draw_string(((320 - screen_get_string_width(ftp_file_transfer, 0.45f, 0.45f)) / 2), 150, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), ftp_file_transfer);

			if (isTransfering)
			{
				screen_draw_rect(50, 140, 220, 3, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
				screen_draw_rect(pBar, 140, 40, 3, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255));

				// Boundary stuff
				screen_draw_rect(0, 140, 50, 3, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
				screen_draw_rect(270, 140, 50, 3, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255)); 
				pBar += 4;
				
				if (pBar >= xlim)
					pBar = 34;
			}
		}

		screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][2], 0.45f, 0.45f)) / 2), 120, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][2]);

		screen_end_frame();

		if (((kHeld & KEY_L) && (kPressed & KEY_R)) || ((kHeld & KEY_R) && (kPressed & KEY_L)))
			captureScreenshot();
	}

	memset(ftp_accepted_connection, 0, 20); // Empty accepted connection address
	memset(ftp_file_transfer, 0, 50); // Empty transfer status
	task_exit();
	ftp_exit();
	DEFAULT_STATE = STATE_HOME;
	updateList(CLEAR);
	displayFiles();
}