#include <3ds.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "menu_ftp.h"
#include "menu_main.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

#define MENUBAR_X_BOUNDARY  0
static int menubar_x = -125;
static char multi_select_dir_old[256];

static void Menu_ControlMenuBar(u32 input)
{
	if (input & KEY_A)
		MENU_DEFAULT_STATE = MENU_STATE_SETTINGS;

	if ((input & KEY_SELECT) || (input & KEY_B))
		MENU_DEFAULT_STATE = MENU_STATE_HOME;
}

static void Menu_HandleMultiSelect(void)
{
	// multi_select_dir can only hold one dir
	strcpy(multi_select_dir_old, cwd);
	//if (strcmp(multi_select_dir_old, multi_select_dir) != 0)
	//	FileOptions_ResetClipboard();

	char path[256];
	File *file = Dirbrowse_GetFileIndex(position);
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);
	strcpy(multi_select_dir, cwd);
			
	if (!multi_select[position])
	{
		multi_select[position] = true;
		multi_select_indices[position] = multi_select_index; // Store the index in the position
		Utils_AppendArr(multi_select_paths[multi_select_index], path, multi_select_index);
		multi_select_index += 1;
	}
	else
	{
		multi_select[position] = false;
		strcpy(multi_select_paths[multi_select_indices[position]], "");
		multi_select_indices[position] = -1;
	}

	Utils_SetMax(&multi_select_index, 0, 50);
	Utils_SetMin(&multi_select_index, 50, 0);
}

static void Menu_ControlHome(u32 input)
{
	if (input & KEY_START)
		longjmp(exitJmp, 1);

	if (input & KEY_SELECT)
	{
		if (MENU_DEFAULT_STATE == MENU_STATE_MENUBAR)
			MENU_DEFAULT_STATE = MENU_STATE_HOME;
		else 
		{
			menubar_x = -125;
			MENU_DEFAULT_STATE = MENU_STATE_MENUBAR;
		}
	}

	if (input & KEY_SELECT)
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_FTP;
		Menu_DisplayFTP();
	}

	if (fileCount > 0)
	{
		if (input & KEY_DUP)
			position--;
		else if (input & KEY_DDOWN)
			position++;

		Utils_SetMax(&position, 0, (fileCount - 1));
		Utils_SetMin(&position, (fileCount - 1), 0);

		if (input & KEY_LEFT)
			position = 0;
		else if (input & KEY_RIGHT)
			position = fileCount - 1;

		// Open options
		if (input & KEY_X)
		{
			if (MENU_DEFAULT_STATE == MENU_STATE_OPTIONS)
				MENU_DEFAULT_STATE = MENU_STATE_HOME;
			else
				MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
		}

		if (input & KEY_Y)
			Menu_HandleMultiSelect();

		if (input & KEY_A)
		{
			wait(5);
			Dirbrowse_OpenFile();
		}
		else if ((strcmp(cwd, ROOT_PATH) != 0) && (input & KEY_B))
		{
			wait(5);
			Dirbrowse_Navigate(-1);
			Dirbrowse_PopulateFiles(true);
		}
	}
}

void Menu_Main(void)
{
	//TouchInfo touchInfo;
	//Touch_Init(&touchInfo);

	Dirbrowse_PopulateFiles(false);
	memset(multi_select, 0, sizeof(multi_select)); // Reset all multi selected items

	while(aptMainLoop())
	{
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, config_dark_theme? BLACK_BG : WHITE);
		C2D_TargetClear(RENDER_BOTTOM, config_dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_TOP);

		Draw_Rect(0, 0, 400, 18, config_dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT); // Status bar
		Draw_Rect(0, 18, 400, 34, config_dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); // Menu bar

		StatusBar_DisplayTime();
		Dirbrowse_DisplayFiles();

		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config_dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config_dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); // Menu bar

		Draw_Image(config_dark_theme? icon_home_dark : icon_home, 0, 0);
		Draw_Image(config_dark_theme? icon_options_dark : icon_options, 25, 0);
		Draw_Image(config_dark_theme? icon_settings_dark : icon_settings, 50, 0);
		Draw_Image(config_dark_theme? icon_ftp_dark : icon_ftp, 75, 0);

		Draw_Image(config_dark_theme? icon_sd_dark : icon_sd, 250, 0);
		Draw_Image(config_dark_theme? icon_secure_dark : icon_secure, 275, 0);
		Draw_Image(icon_search, 300, 0);

		/*else if (MENU_DEFAULT_STATE == MENU_STATE_OPTIONS)
			Menu_DisplayFileOptions();
		else if (MENU_DEFAULT_STATE == MENU_STATE_PROPERTIES)
			Menu_DisplayProperties();
		else if (MENU_DEFAULT_STATE == MENU_STATE_SETTINGS)
			Menu_DisplaySettings();
		else if (MENU_DEFAULT_STATE == MENU_STATE_SORT)
			Menu_DisplaySort();
		else if (MENU_DEFAULT_STATE == MENU_STATE_DIALOG)
			Menu_DisplayDeleteDialog();*/

		Draw_EndFrame();

		hidScanInput();
		//Touch_Process(&touchInfo);
		u32 kDown = hidKeysDown();

		if (MENU_DEFAULT_STATE == MENU_STATE_HOME) 
		{
			Menu_ControlHome(kDown);
			//Menu_TouchHome(touchInfo);
		}
	}
}