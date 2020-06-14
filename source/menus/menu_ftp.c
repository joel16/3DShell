#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
//#include "ftp.h"
#include "bftps.h"
#include "menu_main.h"
#include "status_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

const char* my_basename(const char* path) {
    const char *pLastSlash = path;
    while (*path != '\0') {
        if (*path == '/')
            pLastSlash = path+1;
        path++;
    }
    return pLastSlash;
}

void Menu_DisplayFTP(void) {
        
        bftps_start();

	Result ret = 0;
	char buf[137], buf2[512];
	u32 wifiStatus = 0;

	int position = 0, xlim = 270, heigth = 0;

	//ret = gethostname(hostname, sizeof(hostname));

	if (R_SUCCEEDED(gspLcdInit())) {
		GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_TOP);
		gspLcdExit();
	}

	while(MENU_STATE == MENU_STATE_FTP) {
		//ftp_loop();
		
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); // Menu bar

		ret = ACU_GetWifiStatus(&wifiStatus);
			
		Menu_DrawMenuBar();

		if ((wifiStatus != 0) && R_SUCCEEDED(ret)) {
            Draw_Text(((320 - Draw_GetTextWidth(0.42f, "FTP initialized")) / 2), 37, 0.42f, WHITE, "FTP initialized");
            //snprintf(buf, 137, "IP: %s:5000", R_FAILED(ret)? "Failed to get IP" : hostname);
            snprintf(buf, 137, "%s", bftps_name());

            /* for now I still don't have a list of conected clients, is this really necessary?
            if (strlen(ftp_accepted_connection) != 0)
                    Draw_Text(((320 - Draw_GetTextWidth(0.42f, ftp_accepted_connection)) / 2), 77, 0.42f, WHITE, ftp_accepted_connection);
             */

            Draw_Text(((320 - Draw_GetTextWidth(0.42f, "File browser cannot be accessed at this time.")) / 2), 97, 0.42f, WHITE, "File browser cannot be accesed at this time.");

            const bftps_file_transfer_t* transfersInfo = bftps_file_transfer_retrieve();
            if (transfersInfo) {
                const bftps_file_transfer_t* file = transfersInfo;
                heigth = 0;
                while (file) { // should be limited to the amount of information that can be shown (for now working on two simultaneous)
                    if (file->mode == FILE_SENDING) {
                        float fraction = ((float) file->filePosition / (float) file->fileSize);
                        snprintf(buf2, 512, "Sending: %.2f%%", fraction * (float) 100);
                        //file name should have an elipsis when is to longer
                        Draw_Text(((320 - Draw_GetTextWidth(0.45f, buf2)) / 2), 150 + heigth, 0.45f, WHITE, buf2);
                        Draw_Text(((320 - Draw_GetTextWidth(0.35f, my_basename(file->name))) / 2), 170 + heigth, 0.35f, WHITE, my_basename(file->name));
                        int progress = round(fraction * (float) (xlim - 50));
                        Draw_Rect(50, 140 + heigth, 220, 3, config.dark_theme ? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
                        Draw_Rect(50, 140 + heigth, progress, 3, WHITE);
                    } else {
                        snprintf(buf2, 512, "Receiving: %.2fMB",
                                ((float) file->filePosition / ((float) 1024 * (float) 1024)));
                        //file name should have an elipsis when is to longer
                        Draw_Text(((320 - Draw_GetTextWidth(0.45f, buf2)) / 2), 150 + heigth, 0.45f, WHITE, buf2);
                        Draw_Text(((320 - Draw_GetTextWidth(0.35f, my_basename(file->name))) / 2), 170 + heigth, 0.35f, WHITE, my_basename(file->name));
                        
                        position += 4;
                        if (position >= xlim)
                            position = 34;
                        Draw_Rect(50, 140 + heigth, 220, 3, config.dark_theme ? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
                        Draw_Rect(position, 140 + heigth, 40, 3, WHITE);   
                        // Boundary stuff
                        Draw_Rect(0, 140 + heigth, 50, 3, config.dark_theme ? MENU_BAR_DARK : MENU_BAR_LIGHT);
                        Draw_Rect(270, 140 + heigth, 50, 3, config.dark_theme ? MENU_BAR_DARK : MENU_BAR_LIGHT); 
                    }
                    heigth += 50;
                    file = file->next;
                }                
                bftps_file_transfer_cleanup(transfersInfo);                 
            }

            /*
            if (strlen(ftp_file_transfer) != 0)
                    Draw_Text(((320 - Draw_GetTextWidth(0.42f, ftp_file_transfer)) / 2), 147, 0.42f, WHITE, ftp_file_transfer);

            if (isTransfering) {
                    Draw_Rect(50, 140, 220, 3, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
                    Draw_Rect(pBar, 140, 40, 3, WHITE);

                    // Boundary stuff
                    Draw_Rect(0, 140, 50, 3, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
                    Draw_Rect(270, 140, 50, 3, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); 
                    pBar += 4;
			
                    if (pBar >= xlim)
                            pBar = 34;
            }
             */
        } else {
            Draw_Text(((320 - Draw_GetTextWidth(0.42f, "Failed to initialize FTP.")) / 2), 37, 0.42f, WHITE, "Failed to initialize FTP.");
            snprintf(buf, 18, "WiFi not enabled.");
        }

		Draw_Text(((320 - Draw_GetTextWidth(0.42f, buf)) / 2), 57, 0.42f, WHITE, buf);
		Draw_Text(((320 - Draw_GetTextWidth(0.42f, "Tap the FTP icon to disable the FTP connection.")) / 2), 117, 0.42f, WHITE, "Tap the FTP icon to disable the FTP connection.");

		Draw_EndFrame();

		hidScanInput();
		u32 kDown = hidKeysDown();

		if (((kDown & KEY_TOUCH) && (TouchInRect(73, 0, 97, 20))) || (kDown & KEY_SELECT))
			break;
                // so CPU is not doing unneeded extra work, 100ms should made the UI responsive enough, probably we could increase this to 150 mss
                usleep(100000);
	}

	//memset(ftp_accepted_connection, 0, 20); // Empty accepted connection address
	//memset(ftp_file_transfer, 0, 50); // Empty transfer status
	//ftp_exit();
        bftps_stop();

	if (R_SUCCEEDED(gspLcdInit())) {
		GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_TOP);
		gspLcdExit();
	}

	MENU_STATE = MENU_STATE_HOME;
	Dirbrowse_PopulateFiles(true);
}
