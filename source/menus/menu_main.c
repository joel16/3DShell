#include <3ds.h>

#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "keyboard.h"
#include "menu_delete.h"
#include "menu_file_options.h"
#include "menu_ftp.h"
#include "menu_main.h"
#include "menu_properties.h"
#include "menu_settings.h"
#include "menu_sort.h"
#include "pp2d.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"

struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;

void Menu_Draw_MenuBar(void)
{
	float options_x = pp2d_get_texture_width(TEXTURE_HOME_ICON) + 5;
	float settings_x = pp2d_get_texture_width(TEXTURE_HOME_ICON) + pp2d_get_texture_width(TEXTURE_OPTIONS_ICON) + 15;
	float ftp_x = pp2d_get_texture_width(TEXTURE_HOME_ICON) + pp2d_get_texture_width(TEXTURE_OPTIONS_ICON) + pp2d_get_texture_width(TEXTURE_SETTINGS_ICON) + 25;

	if (MENU_DEFAULT_STATE == MENU_STATE_HOME)
		pp2d_draw_texture(TEXTURE_HOME_ICON_SELECTED, 0, -2);
	else
		pp2d_draw_texture(TEXTURE_HOME_ICON, 0, -2);

	if ((MENU_DEFAULT_STATE == MENU_STATE_OPTIONS) || (MENU_DEFAULT_STATE == MENU_STATE_PROPERTIES))
		pp2d_draw_texture(TEXTURE_OPTIONS_ICON_SELECTED, options_x, 0);
	else
		pp2d_draw_texture(TEXTURE_OPTIONS_ICON, options_x, 0);

	if ((MENU_DEFAULT_STATE == MENU_STATE_SETTINGS) || (MENU_DEFAULT_STATE == MENU_STATE_SORT) || (MENU_DEFAULT_STATE == MENU_STATE_THEMES))
		pp2d_draw_texture(TEXTURE_SETTINGS_ICON_SELECTED, settings_x, 1);
	else
		pp2d_draw_texture(TEXTURE_SETTINGS_ICON, settings_x, 1);

	if (MENU_DEFAULT_STATE == MENU_STATE_FTP)
		pp2d_draw_texture(TEXTURE_FTP_ICON_SELECTED, ftp_x, 0);
	else
		pp2d_draw_texture(TEXTURE_FTP_ICON, ftp_x, 0);

	if (BROWSE_STATE == STATE_SD)
		pp2d_draw_texture(TEXTURE_SD_ICON_SELECTED, (320 - pp2d_get_texture_width(TEXTURE_SD_ICON_SELECTED)) - 55, 0);
	else
		pp2d_draw_texture(TEXTURE_SD_ICON, (320 - pp2d_get_texture_width(TEXTURE_SD_ICON)) - 55, 0);

	if (BROWSE_STATE == STATE_NAND)
		pp2d_draw_texture(TEXTURE_NAND_ICON_SELECTED, (320 - pp2d_get_texture_width(TEXTURE_NAND_ICON_SELECTED)) - 30, 0);
	else
		pp2d_draw_texture(TEXTURE_NAND_ICON, (320 - pp2d_get_texture_width(TEXTURE_NAND_ICON)) - 30, 0);

	pp2d_draw_texture(TEXTURE_SEARCH_ICON, (320 - pp2d_get_texture_width(TEXTURE_SEARCH_ICON)), -2);
}

static void Menu_Main_Controls(void)
{
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
		Screenshot_Capture();

	if ((kDown & KEY_TOUCH) && (touchInRect((320 - pp2d_get_texture_width(TEXTURE_SD_ICON)) - 55, 0, 
		((320 - pp2d_get_texture_width(TEXTURE_SD_ICON)) - 55) + 20, 20))) // SD
	{
		wait(1);
		FS_Write(archive, "/3ds/3DShell/lastdir.txt", START_PATH);
		strcpy(cwd, START_PATH);
			
		BROWSE_STATE = STATE_SD;
			
		FS_CloseArchive(archive);
		FS_OpenArchive(&archive, ARCHIVE_SDMC);

		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}
	else if ((kDown & KEY_TOUCH) && (touchInRect((320 - pp2d_get_texture_width(TEXTURE_NAND_ICON)) - 30, 0, 
		((320 - pp2d_get_texture_width(TEXTURE_NAND_ICON)) - 30) + 20, 20))) // CTR NAND
	{
		wait(1);
		strcpy(cwd, START_PATH);

		BROWSE_STATE = STATE_NAND;

		FS_CloseArchive(archive);
		FS_OpenArchive(&archive, ARCHIVE_NAND_CTR_FS);

		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}

	if ((kDown & KEY_TOUCH) && (touchInRect((320 - pp2d_get_texture_width(TEXTURE_SEARCH_ICON)), 0, 320, 20)))
	{
		char *path = (char *)malloc(256);
		strcpy(path, keyboard_3ds_get(256, "/", "Enter path"));

		if (FS_DirExists(archive, path))
		{
			strcpy(cwd, path);
			Dirlist_PopulateFiles(true);
			Dirlist_DisplayFiles();
		}
		else
			Dirlist_DisplayFiles();

		free(path);
	}

	if ((kDown & KEY_TOUCH) && (touchInRect(0, 0, 22, 20)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_HOME;
	}
	else if ((kDown & KEY_TOUCH) && (touchInRect(23, 0, 47, 20)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
	}
	else if ((kDown & KEY_TOUCH) && (touchInRect(48, 0, 73, 20)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_SETTINGS;
	}
	else if (((kDown & KEY_TOUCH) && (touchInRect(98, 123, 0, 20))) || (kDown & KEY_SELECT))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_FTP;
		Menu_DisplayFTP();
	}

	if (MENU_DEFAULT_STATE == MENU_STATE_OPTIONS)
		Menu_ControlFileOptions(kDown);
	else if (MENU_DEFAULT_STATE == MENU_STATE_PROPERTIES)
		Menu_ControlProperties(kDown);
	else if (MENU_DEFAULT_STATE == MENU_STATE_SETTINGS)
		Menu_ControlSettings(kDown);
	else if (MENU_DEFAULT_STATE == MENU_STATE_SORT)
		Menu_ControlSort(kDown);
	else if (MENU_DEFAULT_STATE == MENU_STATE_DIALOG)
		Menu_ControlDeleteDialog(kDown);
	else if (MENU_DEFAULT_STATE == MENU_STATE_THEMES)
	{
		if (kDown & KEY_A)
		{
			File * file = Dirlist_GetFileIndex(position);

			strcpy(fileName, file->name);

			if ((strncmp(fileName, "default", 7) == 0))
			{
				strcpy(theme_dir, "romfs:/res/drawable");
				strcpy(colour_dir, "/3ds/3DShell/themes/default");

				Theme_SaveConfig(theme_dir, colour_dir);

				wait(1);

				Theme_Load();
				Theme_Reload();
			}
			else if ((strncmp(fileName, "..", 2) != 0) && (file->isDir))
			{
				strcpy(theme_dir, cwd);
				strcpy(colour_dir, cwd);
				
				strcat(theme_dir, fileName);
				strcat(colour_dir, fileName);

				Theme_SaveConfig(theme_dir, colour_dir);
				
				wait(1);

				Theme_Load();
				Theme_Reload();
			}
		}
		if (kDown & KEY_B)
		{
			char buf[250];

			FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
			fscanf(read, "%s", buf);
			fclose(read);

			if (FS_DirExists(archive, buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
				strcpy(cwd, buf);
			else
				strcpy(cwd, START_PATH);

			wait(1);

			MENU_DEFAULT_STATE = MENU_STATE_SETTINGS;
		}
	}
	
	if (fileCount > 0)
	{	
		// Position Decrement
		if (kDown & KEY_DUP)
		{
			// Decrease Position
			if (position > 0)
				position--;

			// Rewind Pointer
			else 
				position = fileCount - 1;
		}

		// Position Increment
		else if (kDown & KEY_DDOWN)
		{
			// Increase Position
			if (position < (fileCount - 1))
				position++;

			// Rewind Pointer
			else 
				position = 0;
		}

		if (kHeld & KEY_CPAD_UP)
		{
			wait(5);

			//scroll_x = 395;
			//scroll_time = osGetTime();

			if (position > 0)
				position--;

			else position = fileCount - 1;
		}

		else if (kHeld & KEY_CPAD_DOWN)
		{
			wait(5);

			//scroll_x = 395;
			//scroll_time = osGetTime();

			if (position < (fileCount - 1))
				position++;

			else position = 0;
		}

		else if (kDown & KEY_A)
		{
			if (MENU_DEFAULT_STATE != MENU_STATE_THEMES)
			{
				wait(1);
				Dirlist_OpenFile(); // Open file/dir
			}
		}

		else if ((strcmp(cwd, ROOT_PATH) != 0) && (kDown & KEY_B))
		{
			if (MENU_DEFAULT_STATE != MENU_STATE_THEMES)
			{
				wait(1);
				Dirlist_Navigate(-1);
				Dirlist_PopulateFiles(true);
				Dirlist_DisplayFiles();
			}	
		}
	}
}

void Menu_Main(void)
{
	Dirlist_PopulateFiles(true);

	while (aptMainLoop())
	{
		pp2d_begin_draw(GFX_TOP, GFX_LEFT);
			pp2d_draw_texture(TEXTURE_BACKGROUND, 0, 0);
			StatusBar_DisplayBar();
			Dirlist_DisplayFiles();
		pp2d_end_draw();

		pp2d_begin_draw(GFX_BOTTOM, GFX_LEFT);
			pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
			pp2d_draw_rectangle(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));

			if (MENU_DEFAULT_STATE == MENU_STATE_OPTIONS)
				Menu_DisplayFileOptions();
			else if (MENU_DEFAULT_STATE == MENU_STATE_PROPERTIES)
				Menu_DisplayProperties();
			else if (MENU_DEFAULT_STATE == MENU_STATE_SETTINGS)
				Menu_DisplaySettings();
			else if (MENU_DEFAULT_STATE == MENU_STATE_SORT)
				Menu_DisplaySort();
			else if (MENU_DEFAULT_STATE == MENU_STATE_DIALOG)
				Menu_DisplayDeleteDialog();

			Menu_Draw_MenuBar();
		pp2d_end_draw();

		hidScanInput();
		Menu_Main_Controls();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
	}
}