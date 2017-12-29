#include <stdio.h>
#include <string.h>

#include "common.h"
#include "file/dirlist.h"
#include "file/file_operations.h"
#include "file/fs.h"
#include "graphics/screen.h"
#include "keyboard.h"
#include "language.h"
#include "main.h"
#include "menus/menu_ftp.h"
#include "menus/menu_main.h"
#include "menus/status_bar.h"
#include "screenshot.h"
#include "task.h"
#include "theme.h"
#include "utils.h"

struct colour Storage_colour;
struct colour TopScreen_colour;
struct colour TopScreen_min_colour;
struct colour TopScreen_bar_colour;
struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;
struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Options_title_text_colour;
struct colour Settings_colour;
struct colour Settings_title_text_colour;
struct colour Settings_text_colour;
struct colour Settings_text_min_colour;

void menu_displayMainMenu(void)
{
	screen_select(GFX_BOTTOM);
	
	screen_draw_rect(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
	screen_draw_rect(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));

	if (DEFAULT_STATE == STATE_HOME)
	{
		screen_draw_texture(TEXTURE_HOME_ICON_SELECTED, -2, -2);
		screen_draw_string(((320 - screen_get_string_width(welcomeMsg, 0.45f, 0.45f)) / 2), 40, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), welcomeMsg);
		screen_draw_string(((320 - screen_get_string_width(currDate, 0.45f, 0.45f)) / 2), 60, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), currDate);
		//screen_draw_string(((320 - screen_get_string_width("Press \uE073 to exit the application.", 0.45f, 0.45f)) / 2), 190, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), "Press \uE073 to exit the application.");
		screen_draw_stringf(2, 225, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), "3DShell %d.%d.%d - %s", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO, __DATE__);
	}
	else
		screen_draw_texture(TEXTURE_HOME_ICON, -2, -2);

	if (DEFAULT_STATE == STATE_OPTIONS)
	{
		screen_draw_texture(TEXTURE_OPTIONS_ICON_SELECTED, 25, 0);

		screen_draw_texture(TEXTURE_OPTIONS, 37, 20);

		screen_draw_rect(37 + (selectionX * 123), 56 + (selectionY * 37), 123, 37, RGBA8(Options_select_colour.r, Options_select_colour.g, Options_select_colour.b, 255));

		screen_draw_string(42, 36, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][0]);
		screen_draw_string(232, 196, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][8]);

		screen_draw_string(47, 72, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][1]);
		screen_draw_string(47, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][3]);
		screen_draw_string(47, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][5]);

		screen_draw_string(170, 72, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][2]);

		if (copyF)
		{
			screen_draw_string(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
			screen_draw_string(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][6]);
		}
		else if (cutF)
		{
			screen_draw_string(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
			screen_draw_string(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][4]);
		}
		else
		{
			screen_draw_string(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][4]);
			screen_draw_string(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][6]);
		}
			
	}
	else
		screen_draw_texture(TEXTURE_OPTIONS_ICON, 25, 0);

	if (DEFAULT_STATE == STATE_SETTINGS)
	{
		screen_draw_rect(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));

		screen_draw_string(10, 30, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_settings[language][0]);

		screen_draw_string(10, 50, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Sort by");
		screen_draw_string(10, 62, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Select from a list of sorting options.");

		screen_draw_string(10, 85, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][5]);
		screen_draw_string(10, 97, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), lang_settings[language][6]);

		screen_draw_string(10, 120, 0.45f, 0.45f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][1]);
		screen_draw_string(10, 132, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), lang_settings[language][2]);

		screen_draw_string(10, 155, 0.45f, 0.45f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][3]);
		screen_draw_stringf(10, 167, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "%s %s", lang_settings[language][4], theme_dir);

		screen_draw_string(10, 190, 0.45f, 0.45f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][7]);
		screen_draw_stringf(10, 202, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "%s", lang_settings[language][8]);

		if (recycleBin)
			screen_draw_texture(TEXTURE_TOGGLE_ON, 280, 90);
		else
			screen_draw_texture(TEXTURE_TOGGLE_OFF, 280, 90);

		if (sysProtection)
			screen_draw_texture(TEXTURE_TOGGLE_ON, 280, 125);
		else
			screen_draw_texture(TEXTURE_TOGGLE_OFF, 280, 125);

		if (isHiddenEnabled)
			screen_draw_texture(TEXTURE_TOGGLE_ON, 280, 195);
		else
			screen_draw_texture(TEXTURE_TOGGLE_OFF, 280, 195);

		screen_draw_texture(TEXTURE_THEME_ICON, 283, 155);
	}

	if (DEFAULT_STATE == STATE_SORT)
	{
		screen_draw_rect(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));

		screen_draw_string(10, 30, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "Sorting options");

		screen_draw_string(10, 50, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "None");
		screen_draw_string(10, 62, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "No sorting method is applied.");

		screen_draw_string(10, 85, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Alphabetical \uE01B");
		screen_draw_string(10, 97, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort alphabetically in ascending order.");

		screen_draw_string(10, 120, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Alphabetical \uE01C");
		screen_draw_string(10, 132, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort alphabetically in descending order.");

		screen_draw_string(10, 155, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Size \uE01B");
		screen_draw_string(10, 167, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort by size (largest first).");

		screen_draw_string(10, 190, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Size \uE01C");
		screen_draw_string(10, 202, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort by size (smallest first).");

		sortBy == 0? screen_draw_texture(TEXTURE_RADIO_ON, 280, 55) : screen_draw_texture(TEXTURE_RADIO_OFF, 280, 55);
		sortBy == 1? screen_draw_texture(TEXTURE_RADIO_ON, 280, 90) : screen_draw_texture(TEXTURE_RADIO_OFF, 280, 90);
		sortBy == 2? screen_draw_texture(TEXTURE_RADIO_ON, 280, 125) : screen_draw_texture(TEXTURE_RADIO_OFF, 280, 125);
		sortBy == 3? screen_draw_texture(TEXTURE_RADIO_ON, 280, 160) : screen_draw_texture(TEXTURE_RADIO_OFF, 280, 160);
		sortBy == 4? screen_draw_texture(TEXTURE_RADIO_ON, 280, 195) : screen_draw_texture(TEXTURE_RADIO_OFF, 280, 195);
	}

	if ((DEFAULT_STATE == STATE_SETTINGS) || (DEFAULT_STATE == STATE_SORT))
		screen_draw_texture(TEXTURE_SETTINGS_ICON_SELECTED, 50, 1);
	else
		screen_draw_texture(TEXTURE_SETTINGS_ICON, 50, 1);

	screen_draw_texture(TEXTURE_UPDATE_ICON, 75, 0);

	screen_draw_texture(TEXTURE_FTP_ICON, 100, 0);

	if (DEFAULT_STATE == STATE_THEME)
		screen_draw_string(((320 - screen_get_string_width(lang_themes[language][0], 0.45f, 0.45f)) / 2), 40, 0.45f, 0.45f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_themes[language][0]);

	if (BROWSE_STATE == STATE_SD)
		screen_draw_texture(TEXTURE_SD_ICON_SELECTED, (320 - screen_get_texture_width(TEXTURE_SD_ICON_SELECTED)) - 55, 0);
	else
		screen_draw_texture(TEXTURE_SD_ICON, (320 - screen_get_texture_width(TEXTURE_SD_ICON)) - 55, 0);

	if (BROWSE_STATE == STATE_NAND)
		screen_draw_texture(TEXTURE_NAND_ICON_SELECTED, (320 - screen_get_texture_width(TEXTURE_NAND_ICON_SELECTED)) - 30, 0);
	else
		screen_draw_texture(TEXTURE_NAND_ICON, (320 - screen_get_texture_width(TEXTURE_NAND_ICON)) - 30, 0);

	screen_draw_texture(TEXTURE_SEARCH_ICON, (320 - screen_get_texture_width(TEXTURE_SEARCH_ICON)), -2);

	screen_select(GFX_TOP);

	screen_draw_texture(TEXTURE_BACKGROUND, 0, 0);

	screen_draw_stringf(84, 28, 0.45f, 0.45f, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), "%.35s", cwd); // Display current path

	drawStatusBar();

	u64 totalStorage = getTotalStorage(BROWSE_STATE? SYSTEM_MEDIATYPE_CTR_NAND : SYSTEM_MEDIATYPE_SD);
	u64 usedStorage = getUsedStorage(BROWSE_STATE? SYSTEM_MEDIATYPE_CTR_NAND : SYSTEM_MEDIATYPE_SD);
	double fill = (((double)usedStorage / (double)totalStorage) * 209.0);

	screen_draw_rect(82, 47, fill, 2, RGBA8(Storage_colour.r, Storage_colour.g, Storage_colour.b, 255)); // Draw storage bar
}

void menu_main(int clearindex)
{
	selectionX = 0, selectionY = 0;
	properties = false, deleteDialog = false;

	if (clearindex != 0)
		updateList(CLEAR);

	//scroll_time = osGetTime();

	while (aptMainLoop())
	{
		// Display file list
		displayFiles();

		hidScanInput();
		hidTouchRead(&touch);

		if (((kHeld & KEY_L) && (kPressed & KEY_R)) || ((kHeld & KEY_R) && (kPressed & KEY_L)))
			captureScreenshot();

		if ((kPressed & KEY_TOUCH) && (touchInRect(0, 22, 0, 20)))
		{
			wait(1);
			DEFAULT_STATE = STATE_HOME;
		}
		else if ((kPressed & KEY_TOUCH) && (touchInRect(23, 47, 0, 20)))
		{
			wait(1);
			DEFAULT_STATE = STATE_OPTIONS;
		}
		else if ((kPressed & KEY_TOUCH) && (touchInRect(48, 73, 0, 20)))
		{
			wait(1);
			DEFAULT_STATE = STATE_SETTINGS;
		}
		/*else if ((kPressed & KEY_TOUCH) && (touchInRect(74, 97, 0, 20)))
		{
			wait(1);
			DEFAULT_STATE = STATE_UPDATE;
		}*/

		if ((kPressed & KEY_TOUCH) && (touchInRect(0, 320, 50, 72)) && (IF_SETTINGS))
		{
			wait(1);
			DEFAULT_STATE = STATE_SORT;
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 90, 110)) && (IF_SETTINGS))
		{
			wait(1);
			if (recycleBin == false)
			{
				recycleBin = true;
				saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			}
			else
			{
				recycleBin = false;
				saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			}
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 125, 145)) && (IF_SETTINGS))
		{
			wait(1);
			if (sysProtection == false)
			{
				sysProtection = true;
				saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			}
			else
			{
				sysProtection = false;
				saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			}
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(283, 303, 155, 175)) && (IF_SETTINGS))
		{
			wait(1);
			DEFAULT_STATE = STATE_THEME;
			strcpy(cwd, "/3ds/3DShell/themes/");
			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 195, 215)) && (IF_SETTINGS))
		{
			wait(1);
			if (isHiddenEnabled == false)
			{
				isHiddenEnabled = true;
				saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			}
			else
			{
				isHiddenEnabled = false;
				saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			}
			updateList(CLEAR);
			displayFiles();
		}

		if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 55, 75)) && (IF_SORT))
		{
			wait(1);
			sortBy = 0;
			saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 90, 110)) && (IF_SORT))
		{
			wait(1);
			sortBy = 1;
			saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 125, 145)) && (IF_SORT))
		{
			wait(1);
			sortBy = 2;
			saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 160, 180)) && (IF_SORT))
		{
			wait(1);
			sortBy = 3;
			saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 195, 215)) && (IF_SORT))
		{
			wait(1);
			sortBy = 4;
			saveConfig(sortBy, recycleBin, sysProtection, isHiddenEnabled);
			updateList(CLEAR);
			displayFiles();
		}

		if ((IF_SORT) && (kPressed & KEY_B))
		{
			wait(1);
			DEFAULT_STATE = STATE_SETTINGS;
		}

		if (((kPressed & KEY_TOUCH) && (touchInRect(98, 123, 0, 20))) || (kPressed & KEY_SELECT))
		{
			wait(1);
			DEFAULT_STATE = STATE_FTP;
		}

		if (DEFAULT_STATE == STATE_FTP)
		{
			menu_displayFTP();
		}

		if ((kPressed & KEY_TOUCH) && (touchInRect((320 - screen_get_texture_width(TEXTURE_SD_ICON)) - 55, 
		((320 - screen_get_texture_width(TEXTURE_SD_ICON)) - 55) + 20, 0, 20))) // SD
		{
			wait(1);
			fsWrite(fsArchive, "/3ds/3DShell/lastdir.txt", START_PATH);
			strcpy(cwd, START_PATH);
			
			BROWSE_STATE = STATE_SD;
			
			closeArchive(fsArchive);
			openArchive(&fsArchive, ARCHIVE_SDMC);

			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect((320 - screen_get_texture_width(TEXTURE_NAND_ICON)) - 30, 
		((320 - screen_get_texture_width(TEXTURE_NAND_ICON)) - 30) + 20, 0, 20))) // CTR NAND
		{
			wait(1);
			strcpy(cwd, START_PATH);

			BROWSE_STATE = STATE_NAND;

			closeArchive(fsArchive);
			openArchive(&fsArchive, ARCHIVE_NAND_CTR_FS);

			updateList(CLEAR);
			displayFiles();
		}

		if ((kPressed & KEY_TOUCH) && (touchInRect(290, 320, 0, 20)))
		{
			strcpy(cwd, keyboard_3ds_get(250, "", "Enter path"));

			if (dirExists(fsArchive, cwd))
			{
				updateList(CLEAR);
				displayFiles();
			}

			else
				displayFiles();
		}

		if (kPressed & KEY_START) // exit
			break;

		if (fileCount > 0)
		{	
			// Position Decrement
			if (kPressed & KEY_DUP)
			{
				// Decrease Position
				if (position > 0)
					position--;

				// Rewind Pointer
				else position = fileCount - 1;

				// Display file list
				displayFiles();
			}

			// Position Increment
			else if (kPressed & KEY_DDOWN)
			{
				// Increase Position
				if (position < (fileCount - 1))
					position++;

				// Rewind Pointer
				else position = 0;

				// Display file list
				displayFiles();
			}

			if (kHeld & KEY_CPAD_UP)
			{
				wait(5);

				//scroll_x = 395;
				//scroll_time = osGetTime();

				if (position > 0)
					position--;

				else position = fileCount - 1;

				displayFiles();
			}

			else if (kHeld & KEY_CPAD_DOWN)
			{
				wait(5);

				//scroll_x = 395;
				//scroll_time = osGetTime();

				if (position < (fileCount - 1))
					position++;

				else position = 0;

				displayFiles();
			}

			else if (kPressed & KEY_A)
			{
				wait(1);

				if (IF_THEME)
				{
					File * file = getFileIndex(position);

					strcpy(fileName, file->name);

					if ((strncmp(fileName, "default", 7) == 0))
					{
						strcpy(theme_dir, "romfs:/res");
						strcpy(colour_dir, "/3ds/3DShell/themes/default");

						saveThemeConfig(theme_dir, colour_dir);

						wait(1);

						loadTheme();
						reloadTheme();
					}
					else if ((strncmp(fileName, "..", 2) != 0) && (file->isDir))
					{
						strcpy(theme_dir, cwd);
						strcpy(colour_dir, cwd);

						strcat(theme_dir, fileName);
						strcat(colour_dir, fileName);

						saveThemeConfig(theme_dir, colour_dir);

						wait(1);

						loadTheme();
						reloadTheme();
					}
				}
				else
					openFile(); // Open file/dir
			}

			else if ((strcmp(cwd, ROOT_PATH) != 0) && (kPressed & KEY_B))
			{
				wait(1);

				if (IF_THEME)
				{
					char buf[250];

					FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
					fscanf(read, "%s", buf);
					fclose(read);

					if (dirExists(fsArchive, buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
						strcpy(cwd, buf);
					else
						strcpy(cwd, START_PATH);

					wait(1);

					DEFAULT_STATE = STATE_SETTINGS;
					updateList(CLEAR);
					displayFiles();
				}

				else
				{
					navigate(-1);
					updateList(CLEAR);
					displayFiles();
				}
			}

			if ((kPressed & KEY_TOUCH) && (touchInRect(37, 282, 179, 217)) && (IF_OPTIONS)) // Cancel
			{
				wait(1);
				copyF = false;
				cutF = false;
				DEFAULT_STATE = STATE_HOME;
			}

			else if ((kHeld & KEY_TOUCH) && (touchInRect(37, 160, 56, 93)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 0;

				wait(1);

				properties = true;
				displayProperties();
			}

			else if ((kPressed & KEY_TOUCH) && (touchInRect(37, 160, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 1;

				wait(1);

				createFolder();
			}

			else if ((kPressed & KEY_TOUCH) && (touchInRect(37, 160, 131, 167)) && (IF_OPTIONS))
			{
				if (((BROWSE_STATE == STATE_NAND) && (!sysProtection)) || (BROWSE_STATE == STATE_SD))
				{
					selectionX = 0;
					selectionY = 2;

					wait(1);

					deleteDialog = true;
					drawDeletionDialog();
				}
			}

			else if ((kPressed & KEY_TOUCH) && (touchInRect(161, 284, 56, 93)) && (IF_OPTIONS))
			{
				if (((BROWSE_STATE == STATE_NAND) && (!sysProtection)) || (BROWSE_STATE == STATE_SD))
				{
					selectionX = 1;
					selectionY = 0;

					wait(1);

					renameFile();
				}
			}

			if ((CAN_COPY) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 1;
				selectionY = 1;
				wait(1);

				if (BROWSE_STATE == STATE_NAND) 
					copyFromNand = true;
				else
					copyFromSD = true;
				
				copy(COPY_KEEP_ON_FINISH);
				copyF = true;
				displayFiles();
			}
			else if (((copyF) && (deleteDialog == false)) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 0;
				wait(1);

				if (paste() == 0)
				{
					copyFromNand = false;
					copyFromSD = false;
					copyF = false;
					updateList(CLEAR);
					displayFiles();
				}
			}

			if ((CAN_CUT) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS))
			{
				if (((BROWSE_STATE == STATE_NAND) && (!sysProtection)) || (BROWSE_STATE == STATE_SD))
				{
					selectionX = 1;
					selectionY = 2;
					wait(1);

					if (BROWSE_STATE == STATE_NAND) 
						copyFromNand = true;
					else
						copyFromSD = true;

					copy(COPY_DELETE_ON_FINISH);
					cutF = true;
					displayFiles();
				}
			}
			else if (((cutF) && (deleteDialog == false)) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS))
			{
				if (((BROWSE_STATE == STATE_NAND) && (!sysProtection)) || (BROWSE_STATE == STATE_SD))
				{
					selectionX = 0;
					selectionY = 0;
					wait(1);

					if (paste() == 0)
					{
						copyFromNand = false;
						copyFromSD = false;
						cutF = false;
						updateList(CLEAR);
						displayFiles();
					}
				}
			}
		}
	}
}