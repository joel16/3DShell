#include <stdlib.h>

#include "C2D_helper.h"
#include "cia.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_main.h"
#include "net.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static int update_dialog_selection = 0;
static bool err = false;
static u32 wifiStatus = 0;
static char version[13];
static float update_confirm_width = 0, update_confirm_height = 0;
static float update_cancel_width = 0, update_cancel_height = 0;

static unsigned int Menu_GetUpdateVerDigit(unsigned int ver) {
	unsigned int ret = 1;

	while (ver/=10)
		ret++;

	return ret;
}

static bool Menu_ValidateUpdate(bool nighlty) {
	char nightly_ver[10];
	int milestone_ver = 0;

	if (nighlty) {
		if (FS_FileExists(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt")) {
			u64 size64 = 0;
			u32 size = 0;

			FS_GetFileSize(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt", &size64);
			size = (u32)size64;
			char *buf = malloc(size + 1);

			if (R_FAILED(FS_Read(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt", size, buf))) {
				free(buf);
				return false;
			}

			buf[size] = '\0';
			sscanf(buf, "%s", nightly_ver);
			free(buf);

			if (strcasecmp(nightly_ver, GITVERSION) != 0) {
				snprintf(version, strlen(nightly_ver), nightly_ver);
				return true;
			}
		}
	}
	else {
		if (FS_FileExists(archive, "/3ds/3DShell/UPDATE_MILESTONE.txt")) {
			u64 size64 = 0;
			u32 size = 0;

			FS_GetFileSize(archive, "/3ds/3DShell/UPDATE_MILESTONE.txt", &size64);
			size = (u32)size64;
			char *buf = malloc(size + 1);

			if (R_FAILED(FS_Read(archive, "/3ds/3DShell/UPDATE_MILESTONE.txt", size, buf))) {
				free(buf);
				return false;
			}

			buf[size] = '\0';
			sscanf(buf, "%d", &milestone_ver);
			free(buf);

			int current_milestone_ver = (VERSION_MAJOR * 100) + (VERSION_MINOR * 10) + VERSION_MICRO;
			
			if (milestone_ver > current_milestone_ver) {
				unsigned int ver_digit = Menu_GetUpdateVerDigit(milestone_ver);
				char arr[ver_digit];
			
				while (ver_digit--) {
					arr[ver_digit] = milestone_ver % 10;
					milestone_ver /= 10;
				}

				snprintf(version, 13, "v%d.%d.%d", arr[0], arr[1], arr[2]);
				return true;
			}
		}	
	}

	return false;
}

static void Menu_InstallUpdate(void) {
	if (wifiStatus != 0) {
		if (envIsHomebrew()) {
			if (FS_FileExists(archive, "/3ds/3DShell/3DShell.3dsx"))
				FS_RemoveFile(archive, "/3ds/3DShell/3DShell.3dsx");

			Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/3DShell.3dsx", "/3ds/3DShell/3DShell.3dsx");
			longjmp(exitJmp, 1);
		}
		else {
			Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/3DShell.cia", "/3ds/3DShell/3DShell.cia");
			CIA_InstallTitle("/3ds/3DShell/3DShell.cia", MEDIATYPE_SD, true);
		}
	}
}

void Menu_DisplayUpdate(void) {
	float err_width = ((320 - Draw_GetTextWidth(0.45f, "No updates available.")) / 2);

	Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
	Draw_Rect(0, 20, 320, 220, config.dark_theme? BLACK_BG : WHITE);

	Menu_DrawMenuBar();

	Draw_Rect(0, 20, 400, 35, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
	Draw_Text(10, 30, 0.48f, WHITE, "Update Center");

	ACU_GetWifiStatus(&wifiStatus);

	if (wifiStatus == 0)
		Draw_Text(((320 - Draw_GetTextWidth(0.48f, "WiFi not enabled.")) / 2), 60, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "WiFi not enabled.");
	else if (err)
		Draw_Text(err_width, 60, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "No updates available.");

	Draw_Text(122, 85, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Nightly builds");
	Draw_Text(52, 97, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Untested builds that may contain bugs.");

	Draw_Rect(106, 117, (Draw_GetTextWidth(0.45, "Check for updates") + 10), 20, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Rect(110 - 3, 118, (Draw_GetTextWidth(0.45, "Check for updates") + 8), 18, config.dark_theme? BLACK_BG : WHITE);
	Draw_Rect(110 - 2, 119, (Draw_GetTextWidth(0.45, "Check for updates") + 6), 16, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Text(110, 120, 0.45f, config.dark_theme? BLACK_BG : WHITE, "Check for updates");

	Draw_Text(115, 155, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Milestone builds");
	Draw_Text(102, 167, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Official release builds.");
	
	Draw_Rect(106, 187, (Draw_GetTextWidth(0.45, "Check for updates") + 10), 20, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Rect(110 - 3, 188, (Draw_GetTextWidth(0.45, "Check for updates") + 8), 18, config.dark_theme? BLACK_BG : WHITE);
	Draw_Rect(110 - 2, 189, (Draw_GetTextWidth(0.45, "Check for updates") + 6), 16, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Text(110, 190, 0.45f, config.dark_theme? BLACK_BG : WHITE, "Check for updates");
}

void Menu_DisplayUpdate2(void) {
	float text_width = 0, text2_width = 0;

	Draw_GetTextSize(0.45f, &text_width, NULL, "This action cannot be undone.");
	Draw_GetTextSize(0.45f, &text2_width, NULL, "Do you wish to update?");
	Draw_GetTextSize(0.45f, &update_confirm_width, &update_confirm_height, "YES");
	Draw_GetTextSize(0.45f, &update_cancel_width, &update_cancel_height, "NO");

	Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

	Draw_Textf(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "New update: %s", version);

	Draw_Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "This action cannot be undone.");
	Draw_Text(((320 - (text2_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 55, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Do you wish to update?");

	if (update_dialog_selection == 0)
		Draw_Rect((288 - update_cancel_width) - 5, (159 - update_cancel_height) - 5, update_cancel_width + 10, update_cancel_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (update_dialog_selection == 1)
		Draw_Rect((248 - (update_confirm_width)) - 5, (159 - update_confirm_height) - 5, update_confirm_width + 10, update_confirm_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	Draw_Text(248 - (update_confirm_width), (159 - update_confirm_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
	Draw_Text(288 - update_cancel_width, (159 - update_cancel_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");
}

void Menu_ControlUpdate(u32 input) {
	if ((input & KEY_TOUCH) && (TouchInRect(106, 117, (106) + (Draw_GetTextWidth(0.45, "Check for updates") + 10), 137))) {
		if (wifiStatus != 0) {
			Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/UPDATE_NIGHTLY.txt", "/3ds/3DShell/UPDATE_NIGHTLY.txt");
		
			if (Menu_ValidateUpdate(true))
				MENU_STATE = MENU_STATE_UPDATE_2;
			else
				err = true;
		}
	}

	else if ((input & KEY_TOUCH) && (TouchInRect(106, 187, (106) + (Draw_GetTextWidth(0.45, "Check for updates") + 10), 207))) {
		if (wifiStatus != 0) {
			Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/UPDATE_MILESTONE.txt", "/3ds/3DShell/UPDATE_MILESTONE.txt");
			if (Menu_ValidateUpdate(false))
				MENU_STATE = MENU_STATE_UPDATE_2;
			else
				err = true;
		}
	}
}

void Menu_ControlUpdate2(u32 input) {
	if (input & KEY_RIGHT)
		update_dialog_selection++;
	else if (input & KEY_LEFT)
		update_dialog_selection--;

	Utils_SetMax(&update_dialog_selection, 0, 1);
	Utils_SetMin(&update_dialog_selection, 1, 0);

	if (input & KEY_B) {
		update_dialog_selection = 0;
		memset(version, 0, 10);
		MENU_STATE = MENU_STATE_UPDATE;
	}

	if (input & KEY_A) {
		if (update_dialog_selection == 1)
			Menu_InstallUpdate();
		else {
			update_dialog_selection = 0;
			MENU_STATE = MENU_STATE_UPDATE;
		}

		memset(version, 0, 10);
		update_dialog_selection = 0;
	}

	if (TouchInRect((288 - update_cancel_width) - 5, (159 - update_cancel_height) - 5, ((288 - update_cancel_width) - 5) + update_cancel_width + 10, ((159 - update_cancel_height) - 5) + update_cancel_height + 10)) {
		update_dialog_selection = 0;

		if (input & KEY_TOUCH) {
			update_dialog_selection = 0;
			memset(version, 0, 10);
			MENU_STATE = MENU_STATE_UPDATE;
		}
	}
	else if (TouchInRect((248 - (update_confirm_height)) - 5, (159 - update_confirm_height) - 5, ((248 - (update_confirm_height)) - 5) + update_confirm_height + 10, ((159 - update_confirm_height) - 5) + update_confirm_height + 10)) {
		update_dialog_selection = 1;

		if (input & KEY_TOUCH) {
			memset(version, 0, 10);
			Menu_InstallUpdate();
		}
	}
}
