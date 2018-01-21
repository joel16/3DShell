#include <stdio.h>
#include <string.h>

#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "ftp.h"
#include "language.h"
#include "menu_ftp.h"
#include "menu_main.h"
#include "pp2d.h"
#include "status_bar.h"
#include "textures.h"
#include "theme.h"
#include "utils.h"

struct colour Storage_colour;
struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;

void Menu_DisplayFTP(void)
{
	ftp_init();

	touchPosition touch;
	
	char buf[25];
	u32 wifiStatus = 0;

	int pBar = 0, xlim = 270;

	while((aptMainLoop()) && (MENU_DEFAULT_STATE == MENU_STATE_FTP))
	{
		ftp_loop();

		ACU_GetWifiStatus(&wifiStatus);

		pp2d_begin_draw(GFX_BOTTOM, GFX_LEFT);

			pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
			pp2d_draw_rectangle(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
			
			Menu_Draw_MenuBar();

			if (!(wifiStatus))
			{
				pp2d_draw_text(((320 - pp2d_get_text_width(lang_ftp[language][3], 0.45f, 0.45f)) / 2), 40, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][3]);
				sprintf(buf, lang_ftp[language][4]);
			}
			else
			{
				pp2d_draw_text(((320 - pp2d_get_text_width(lang_ftp[language][0], 0.45f, 0.45f)) / 2), 40, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][0]);

				u32 ip = gethostid();
				sprintf(buf, "IP: %lu.%lu.%lu.%lu:5000", ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF);

				pp2d_draw_text(((320 - pp2d_get_text_width(buf, 0.45f, 0.45f)) / 2), 60, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), buf);

				if (strlen(ftp_accepted_connection) != 0)
					pp2d_draw_text(((320 - pp2d_get_text_width(ftp_accepted_connection, 0.45f, 0.45f)) / 2), 80, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), ftp_accepted_connection);

				pp2d_draw_text(((320 - pp2d_get_text_width(lang_ftp[language][1], 0.45f, 0.45f)) / 2), 100, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][1]);

				if (strlen(ftp_file_transfer) != 0)
					pp2d_draw_text(((320 - pp2d_get_text_width(ftp_file_transfer, 0.45f, 0.45f)) / 2), 150, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), ftp_file_transfer);

				if (isTransfering)
				{
					pp2d_draw_rectangle(50, 140, 220, 3, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
					pp2d_draw_rectangle(pBar, 140, 40, 3, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255));

					// Boundary stuff
					pp2d_draw_rectangle(0, 140, 50, 3, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
					pp2d_draw_rectangle(270, 140, 50, 3, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255)); 
					pBar += 4;
				
					if (pBar >= xlim)
						pBar = 34;
				}
			}

			pp2d_draw_text(((320 - pp2d_get_text_width(lang_ftp[language][2], 0.45f, 0.45f)) / 2), 120, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][2]);

		pp2d_end_draw();

		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_SELECT)
			break;
	}

	memset(ftp_accepted_connection, 0, 20); // Empty accepted connection address
	memset(ftp_file_transfer, 0, 50); // Empty transfer status
	ftp_exit();
	MENU_DEFAULT_STATE = MENU_STATE_HOME;
	Dirlist_PopulateFiles(true);
	Dirlist_DisplayFiles();
}