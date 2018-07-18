#include <stdio.h>
#include <string.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "ftp.h"
#include "menu_ftp.h"
#include "menu_main.h"
#include "status_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

void Menu_DisplayFTP(void)
{
	ftp_init();

	touchPosition touch;

	char buf[25];
	u32 wifiStatus = 0;

	int pBar = 0, xlim = 270;

	while(MENU_STATE == MENU_STATE_FTP)
	{
		ftp_loop();
		
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_BOTTOM, config_dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config_dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config_dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); // Menu bar

		ACU_GetWifiStatus(&wifiStatus);
			
		Menu_DrawMenuBar();

		if (wifiStatus == 0)
		{
			Draw_Text(((320 - Draw_GetTextWidth(0.48f, "Failed to initialize FTP.")) / 2), 40, 0.48f, WHITE, "Failed to initialize FTP.");
			sprintf(buf, "WiFi not enabled.");
		}
		else
		{
			Draw_Text(((320 - Draw_GetTextWidth(0.48f, "FTP initialized")) / 2), 40, 0.48f, WHITE, "FTP initialized");
			
			u32 ip = gethostid();
			sprintf(buf, "IP: %lu.%lu.%lu.%lu:5000", ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF);

			Draw_Text(((320 - Draw_GetTextWidth(0.48f, buf)) / 2), 60, 0.48f, WHITE, buf);

			if (strlen(ftp_accepted_connection) != 0)
				Draw_Text(((320 - Draw_GetTextWidth(0.48f, ftp_accepted_connection)) / 2), 80, 0.48f, WHITE, ftp_accepted_connection);

			Draw_Text(((320 - Draw_GetTextWidth(0.48f, "File browser cannot be accesed at this time.")) / 2), 100, 0.48f, WHITE, "File browser cannot be accesed at this time.");

			if (strlen(ftp_file_transfer) != 0)
				Draw_Text(((320 - Draw_GetTextWidth(0.45f, ftp_file_transfer)) / 2), 150, 0.45f, WHITE, ftp_file_transfer);

			if (isTransfering)
			{
				Draw_Rect(50, 140, 220, 3, config_dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
				Draw_Rect(pBar, 140, 40, 3, WHITE);

				// Boundary stuff
				Draw_Rect(0, 140, 50, 3, config_dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
				Draw_Rect(270, 140, 50, 3, config_dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); 
				pBar += 4;
			
				if (pBar >= xlim)
					pBar = 34;
			}
		}

		Draw_Text(((320 - Draw_GetTextWidth(0.48f, "Tap the FTP icon to disable the FTP connection.")) / 2), 120, 0.48f, WHITE, "Tap the FTP icon to disable the FTP connection.");

		Draw_EndFrame();

		hidScanInput();
		u32 kDown = hidKeysDown();

		if (((kDown & KEY_TOUCH) && (TouchInRect(73, 0, 97, 20))) || (kDown & KEY_SELECT))
			break;
	}

	memset(ftp_accepted_connection, 0, 20); // Empty accepted connection address
	memset(ftp_file_transfer, 0, 50); // Empty transfer status
	ftp_exit();
	MENU_STATE = MENU_STATE_HOME;
	Dirbrowse_PopulateFiles(true);
}