#include <stdlib.h>

#include "C2D_helper.h"
#include "cia.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_main.h"
#include "menu_update.h"
#include "net.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static int update_dialog_selection = 0;
static bool err = false;
char ver[10];

static bool Menu_ValidateUpdate(bool nighlty)
{
	if (nighlty)
	{
		if (FS_FileExists(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt"))
		{
			u64 size64 = 0;
			u32 size = 0;

			FS_GetFileSize(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt", &size64);
			size = (u32)size64;
			char *buf = (char *)malloc(size + 1);

			if (R_FAILED(FS_Read(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt", size, buf)))
			{
				free(buf);
				return false;
			}

			buf[size] = '\0';
			sscanf(buf, "%s", ver);
			free(buf);

			if (strcmp(ver, GITVERSION) != 0)
				return true;

			return false;
		}
	}
}

void Menu_DisplayUpdate(void)
{
	float err_width = ((320 - Draw_GetTextWidth(0.45f, "No updates available.")) / 2);

	Draw_Rect(0, 0, 320, 20, config_dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
	Draw_Rect(0, 20, 320, 220, config_dark_theme? BLACK_BG : WHITE);

	Menu_DrawMenuBar();

	Draw_Rect(0, 20, 400, 35, config_dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
	Draw_Text(10, 30, 0.48f, WHITE, "Update Center");

	if (err)
		Draw_Text(err_width, 60, 0.45f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "No updates available.");

	Draw_Text(122, 85, 0.45f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Nightly builds");
	Draw_Text(52, 97, 0.45f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Untested builds that may contain bugs.");

	Draw_Rect(106, 117, (Draw_GetTextWidth(0.45, "Check for updates") + 10), 20, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Rect(110 - 3, 118, (Draw_GetTextWidth(0.45, "Check for updates") + 8), 18, config_dark_theme? BLACK_BG : WHITE);
	Draw_Rect(110 - 2, 119, (Draw_GetTextWidth(0.45, "Check for updates") + 6), 16, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Text(110, 120, 0.45f, config_dark_theme? BLACK_BG : WHITE, "Check for updates");

	Draw_Text(115, 155, 0.45f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Milestone builds");
	Draw_Text(102, 167, 0.45f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Official release builds.");
	
	Draw_Rect(106, 187, (Draw_GetTextWidth(0.45, "Check for updates") + 10), 20, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Rect(110 - 3, 188, (Draw_GetTextWidth(0.45, "Check for updates") + 8), 18, config_dark_theme? BLACK_BG : WHITE);
	Draw_Rect(110 - 2, 189, (Draw_GetTextWidth(0.45, "Check for updates") + 6), 16, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
	Draw_Text(110, 190, 0.45f, config_dark_theme? BLACK_BG : WHITE, "Check for updates");
}

void Menu_DisplayUpdate2(void)
{
	float text_width = 0, text2_width = 0;;
	float update_confirm_width = 0, update_confirm_height = 0;
	float update_cancel_width = 0, update_cancel_height = 0;

	Draw_GetTextSize(0.45f, &text_width, NULL, "This action cannot be undone.");
	Draw_GetTextSize(0.45f, &text2_width, NULL, "Do you wish to update?");
	Draw_GetTextSize(0.45f, &update_confirm_width, &update_confirm_height, "YES");
	Draw_GetTextSize(0.45f, &update_cancel_width, &update_cancel_height, "NO");

	Draw_Image(config_dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

	Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Confirm update");

	Draw_Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40, 0.45f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "This action cannot be undone.");
	Draw_Text(((320 - (text2_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 55, 0.45f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Do you wish to update?");

	if (update_dialog_selection == 0)
		Draw_Rect((288 - update_cancel_width) - 5, (159 - update_cancel_height) - 5, update_cancel_width + 10, update_cancel_height + 10, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (update_dialog_selection == 1)
		Draw_Rect((248 - (update_confirm_width)) - 5, (159 - update_confirm_height) - 5, update_confirm_width + 10, update_confirm_height + 10, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	Draw_Text(248 - (update_confirm_width), (159 - update_confirm_height), 0.45f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
	Draw_Text(288 - update_cancel_width, (159 - update_cancel_height), 0.45f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");
}

void Menu_ControlUpdate(u32 input)
{
	if ((input & KEY_TOUCH) && (TouchInRect(106, 117, (106) + (Draw_GetTextWidth(0.45, "Check for updates") + 10), 137)))
	{
		wait(1);
		Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/UPDATE_NIGHTLY.txt", "/3ds/3DShell/UPDATE_NIGHTLY.txt");
		
		if (Menu_ValidateUpdate(true))
			MENU_STATE = MENU_STATE_UPDATE_2;
		else
			err = true;
	}

	else if ((input & KEY_TOUCH) && (TouchInRect(106, 187, (106) + (Draw_GetTextWidth(0.45, "Check for updates") + 10), 207)))
	{
		wait(1);
		Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/UPDATE_MILESTONE.txt", "/3ds/3DShell/UPDATE_MILESTONE.txt");
		if (Menu_ValidateUpdate(false))
			MENU_STATE = MENU_STATE_UPDATE_2;
		else
			err = true;
	}
}

void Menu_ControlUpdate2(u32 input)
{
	if (input & KEY_RIGHT)
		update_dialog_selection++;
	else if (input & KEY_LEFT)
		update_dialog_selection--;

	Utils_SetMax(&update_dialog_selection, 0, 1);
	Utils_SetMin(&update_dialog_selection, 1, 0);

	if (input & KEY_B)
	{
		wait(1);
		update_dialog_selection = 0;
		MENU_STATE = MENU_STATE_UPDATE;
	}

	if (input & KEY_A)
	{
		if (update_dialog_selection == 1)
		{
			if (envIsHomebrew())
			{
				if (FS_FileExists(archive, "/3ds/3DShell/3DShell.3dsx"))
					FS_Remove(archive, "/3ds/3DShell/3DShell.3dsx");
				Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/3DShell.3dsx", "/3ds/3DShell/3DShell.3dsx");
				longjmp(exitJmp, 1);
			}
			else
			{
				Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/3DShell.cia", "/3ds/3DShell/3DShell.cia");
				CIA_InstallTitle("/3ds/3DShell/3DShell.cia", MEDIATYPE_SD, true);
			}
		}
		else
		{
			wait(1);
			update_dialog_selection = 0;
			MENU_STATE = MENU_STATE_UPDATE;
		}

		update_dialog_selection = 0;
	}
}
