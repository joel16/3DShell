#include "common.h"
#include "file/dirlist.h"
#include "file/file_operations.h"
#include "file/fs.h"
#include "graphics/screen.h"
#include "keyboard.h"
#include "language.h"
#include "menus/menu_ftp.h"
#include "menus/menu_main.h"
#include "screenshot.h"
#include "task.h"
#include "theme.h"
#include "utils.h"

void menu_main(int clearindex)
{
	selectionX = 0, selectionY = 0;
	properties = false, deleteDialog = false;

	if (clearindex != 0)
		updateList(CLEAR);

	while (aptMainLoop())
	{
		// Display file list
		displayFiles();

		hidScanInput();
		hidTouchRead(&touch);

		if ((kHeld & KEY_L) && (kHeld & KEY_R))
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

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 50, 72)) && (IF_SETTINGS))
		{
			wait(1);
			if (recycleBin == false)
			{
				recycleBin = true;
				saveConfig(recycleBin, sysProtection, isHiddenEnabled);
			}
			else
			{
				recycleBin = false;
				saveConfig(recycleBin, sysProtection, isHiddenEnabled);
			}
		}

		if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 90, 112)) && (IF_SETTINGS))
		{
			wait(1);
			if (sysProtection == false)
			{
				sysProtection = true;
				saveConfig(recycleBin, sysProtection, isHiddenEnabled);
			}
			else
			{
				sysProtection = false;
				saveConfig(recycleBin, sysProtection, isHiddenEnabled);
			}
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(283, 303, 125, 145)) && (IF_SETTINGS))
		{
			wait(1);
			DEFAULT_STATE = STATE_THEME;
			strcpy(cwd, "/3ds/data/3DShell/themes/");
			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(280, 320, 170, 192)) && (IF_SETTINGS))
		{
			wait(1);
			if (isHiddenEnabled == false)
			{
				isHiddenEnabled = true;
				saveConfig(recycleBin, sysProtection, isHiddenEnabled);
			}
			else
			{
				isHiddenEnabled = false;
				saveConfig(recycleBin, sysProtection, isHiddenEnabled);
			}
			updateList(CLEAR);
			displayFiles();
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

		if ((kPressed & KEY_TOUCH) && (touchInRect(0, 320, 40, 54)))
		{
			wait(1);
			
			fsWrite("/3ds/data/3DShell/lastdir.txt", START_PATH);
			strcpy(cwd, START_PATH);
			BROWSE_STATE = STATE_SD;
			closeArchive(fsArchive);
			openArchive(&fsArchive, ARCHIVE_SDMC);

			updateList(CLEAR);
			displayFiles();
		}

		else if ((kPressed & KEY_TOUCH) && (touchInRect(148, 173, 0, 20))) // SD
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
				wait(6);

				if (position > 0)
					position--;

				else position = fileCount - 1;

				displayFiles();
			}

			else if (kHeld & KEY_CPAD_DOWN)
			{
				wait(6);

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
						strcpy(colour_dir, "/3ds/data/3DShell/themes/default");

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

					FILE * read = fopen("/3ds/data/3DShell/lastdir.txt", "r");
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
				copy(COPY_KEEP_ON_FINISH);
				copyF = true;
				displayFiles();
			}
			else if (((copyF == true) && (deleteDialog == false)) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 0;
				wait(1);

				if (paste() == 0)
				{
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
					copy(COPY_DELETE_ON_FINISH);
					cutF = true;
					displayFiles();
				}
			}
			else if (((cutF == true) && (deleteDialog == false)) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS))
			{
				if (((BROWSE_STATE == STATE_NAND) && (!sysProtection)) || (BROWSE_STATE == STATE_SD))
				{
					selectionX = 0;
					selectionY = 0;
					wait(1);

					if (paste() == 0)
					{
						cutF = false;
						updateList(CLEAR);
						displayFiles();
					}
				}
			}
		}
	}
}