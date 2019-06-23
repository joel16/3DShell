#include <stdlib.h>
#include <3ds.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "keyboard.h"
#include "menu_ftp.h"
#include "menu_fileoptions.h"
#include "menu_main.h"
#include "menu_update.h"
#include "menu_settings.h"
#include "status_bar.h"
#include "screenshot.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

int multi_select_index;
bool multi_select[256];
int multi_select_indices[256];
char multi_select_dir[256];
char multi_select_paths[51][256];
static char multi_select_dir_old[256];

static void Menu_HandleMultiSelect(void) {
	// multi_select_dir can only hold one dir
	strcpy(multi_select_dir_old, cwd);
	if (strcmp(multi_select_dir_old, multi_select_dir) != 0)
		FileOptions_ResetClipboard();

	char path[256];
	File *file = Dirbrowse_GetFileIndex(position);
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);
	strcpy(multi_select_dir, cwd);
			
	if (!multi_select[position]) {
		multi_select[position] = true;
		multi_select_indices[position] = multi_select_index; // Store the index in the position
		Utils_AppendArr(multi_select_paths[multi_select_index], path, multi_select_index);
		multi_select_index += 1;
	}
	else {
		multi_select[position] = false;
		strcpy(multi_select_paths[multi_select_indices[position]], "");
		multi_select_indices[position] = -1;
	}

	Utils_SetMax(&multi_select_index, 0, 50);
	Utils_SetMin(&multi_select_index, 50, 0);
}

static void Menu_ControlHome(u32 input) {
	if (file_count > 0) {
		if (input & KEY_DUP)
			position--;
		else if (input & KEY_DDOWN)
			position++;

		if (hidKeysHeld() & KEY_CPAD_UP) {
			wait(5);
			position--;
		}
		else if (hidKeysHeld() & KEY_CPAD_DOWN) {
			wait(5);
			position++;
		}

		Utils_SetMax(&position, 0, (file_count - 1));
		Utils_SetMin(&position, (file_count - 1), 0);

		if (input & KEY_LEFT)
			position = 0;
		else if (input & KEY_RIGHT)
			position = file_count - 1;

		// Open options
		if (input & KEY_X) {
			if (MENU_STATE == MENU_STATE_FILEOPTIONS)
				MENU_STATE = MENU_STATE_HOME;
			else
				MENU_STATE = MENU_STATE_FILEOPTIONS;
		}

		if (input & KEY_Y)
			Menu_HandleMultiSelect();

		if (input & KEY_A) {
			Dirbrowse_OpenFile();
		}
		else if ((strcmp(cwd, ROOT_PATH) != 0) && (input & KEY_B)) {
			Dirbrowse_Navigate(true);
			Dirbrowse_PopulateFiles(true);
		}
	}
}

void Menu_DrawMenuBar(void) {
	Draw_Image(MENU_STATE == MENU_STATE_HOME? icon_home_overlay : (config.dark_theme? icon_home_dark : icon_home), 0, -2);
	Draw_Image((MENU_STATE == MENU_STATE_FILEOPTIONS) || (MENU_STATE == MENU_STATE_PROPERTIES) || (MENU_STATE == MENU_STATE_DELETE)? 
		icon_options_overlay : (config.dark_theme? icon_options_dark : icon_options), 25, 0);
	Draw_Image((MENU_STATE == MENU_STATE_SETTINGS) || (MENU_STATE == MENU_STATE_SORT) || (MENU_STATE == MENU_STATE_ABOUT)? 
		icon_settings_overlay : (config.dark_theme? icon_settings_dark : icon_settings), 50, 0);
	Draw_Image(MENU_STATE == MENU_STATE_FTP? icon_ftp_overlay : (config.dark_theme? icon_ftp_dark : icon_ftp), 75, 0);
	Draw_Image((MENU_STATE == MENU_STATE_UPDATE) || (MENU_STATE == MENU_STATE_UPDATE_2)? icon_updates_overlay : 
		(config.dark_theme? icon_updates_dark : icon_updates), 100, 1);
	Draw_Image(BROWSE_STATE == BROWSE_STATE_SD? icon_sd_overlay : (config.dark_theme? icon_sd_dark : icon_sd), 250, 0);
	Draw_Image(BROWSE_STATE == BROWSE_STATE_NAND? icon_secure_overlay : (config.dark_theme? icon_secure_dark : icon_secure), 275, 0);
	Draw_Image(icon_search, 300, 0);
}

void Menu_ControlMenuBar(u32 input) {
	if ((input & KEY_TOUCH) && (TouchInRect(0, 0, 22, 20))) {
		MENU_STATE = MENU_STATE_HOME;
	}
	else if ((input & KEY_TOUCH) && (TouchInRect(23, 0, 47, 20))) {
		MENU_STATE = MENU_STATE_FILEOPTIONS;
	}
	else if ((input & KEY_TOUCH) && (TouchInRect(48, 0, 72, 20))) {
		MENU_STATE = MENU_STATE_SETTINGS;
	}
	else if (((input & KEY_TOUCH) && (TouchInRect(73, 0, 97, 20))) || (input & KEY_SELECT)) {
		MENU_STATE = MENU_STATE_FTP;
		Menu_DisplayFTP();
	}
	else if ((input & KEY_TOUCH) && (TouchInRect(98, 0, 122, 20))) {
		MENU_STATE = MENU_STATE_UPDATE;
	}
}

static void Menu_ControlBrowseOptions(u32 input) {
	if ((input & KEY_TOUCH) && (TouchInRect(247, 0, 272, 20))) { // SD
		BROWSE_STATE = BROWSE_STATE_SD;
		archive = sdmc_archive;
		Config_GetLastDirectory();

		Dirbrowse_PopulateFiles(true);
	}
	else if ((input & KEY_TOUCH) && (TouchInRect(273, 0, 292, 20))) { // CTR NAND
		strcpy(cwd, START_PATH);

		BROWSE_STATE = BROWSE_STATE_NAND;
		archive = nand_archive;

		Dirbrowse_PopulateFiles(true);
	}

	if ((input & KEY_TOUCH) && (TouchInRect(293, 0, 320, 20))) {
		char path[256] = {0};
		strcpy(path, Keyboard_GetText("/", "Enter path"));

		const int len = strlen(path);
		if (len > 0 && path[len -1] != '/')
			strcat(path, "/");

		if (FS_DirExists(archive, path)) {
			strcpy(cwd, path);
			Dirbrowse_PopulateFiles(true);
		}
	}
}

void Menu_Main(void) {
	Dirbrowse_PopulateFiles(false);
	memset(multi_select, 0, sizeof(multi_select)); // Reset all multi selected items

	while(aptMainLoop()) {
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, config.dark_theme? BLACK_BG : WHITE);
		C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_TOP);

		Draw_Rect(0, 0, 400, 18, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT); // Status bar
		Draw_Rect(0, 18, 400, 34, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); // Menu bar

		StatusBar_DisplayTime();
		Dirbrowse_DisplayFiles();

		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar

		Menu_DrawMenuBar();

		hidScanInput();
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		Menu_ControlMenuBar(kDown);
		
		if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
			Screenshot_Capture();

		if (MENU_STATE == MENU_STATE_HOME) {
			Menu_ControlHome(kDown);
			Menu_ControlBrowseOptions(kDown);
		}
		else if (MENU_STATE == MENU_STATE_FILEOPTIONS) {
			Menu_DisplayFileOptions();
			Menu_ControlFileOptions(kDown);
		}
		else if (MENU_STATE == MENU_STATE_PROPERTIES) {
			Menu_DisplayProperties();
			Menu_ControlProperties(kDown);
		}
		else if (MENU_STATE == MENU_STATE_DELETE) {
			Menu_DisplayDeleteDialog();
			Menu_ControlDeleteDialog(kDown);
		}
		else if (MENU_STATE == MENU_STATE_SETTINGS) {
			Menu_DisplaySettings();
			Menu_ControlSettings(kDown);
		}
		else if (MENU_STATE == MENU_STATE_SORT) {
			Menu_DisplaySortSettings();
			Menu_ControlSortSettings(kDown);
		}
		else if (MENU_STATE == MENU_STATE_ABOUT) {
			Menu_DisplayAbout();
			Menu_ControlAbout(kDown);
		}
		else if (MENU_STATE == MENU_STATE_UPDATE) {
			Menu_DisplayUpdate();
			Menu_ControlUpdate(kDown);
		}
		else if (MENU_STATE == MENU_STATE_UPDATE_2) {
			Menu_DisplayUpdate2();
			Menu_ControlUpdate2(kDown);
		}

		Draw_EndFrame();

		if (kDown & KEY_START)
			longjmp(exitJmp, 1);
	}
}
