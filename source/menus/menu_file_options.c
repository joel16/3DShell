#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "keyboard.h"
#include "language.h"
#include "menu_properties.h"
#include "menu_settings.h"
#include "pp2d.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"
#include "utils.h"

struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Settings_title_text_colour;

static int selectionX, selectionY;
static bool copyF, cutF;

static Result FileOptions_CreateFolder(void)
{
	char * buf = (char *)malloc(256);
	strcpy(buf, keyboard_3ds_get(256, "", "Enter name"));

	if (strncmp(buf, "", 1) == 0)
		return -1;

	char path[500];
	strcpy(path, cwd);
	strcat(path, buf);
	free(buf);

	FS_RecursiveMakeDir(archive, path);

	Dirlist_PopulateFiles(true);
	return 0;
}

static Result FileOptions_RenameFile(void)
{
	Result ret = 0;
	File * file = Dirlist_GetFileIndex(position);

	if (file == NULL)
		return -1;

	if (strncmp(file->name, "..", 2) == 0)
		return -2;

	char oldPath[500], newPath[500];

	char * buf = (char *)malloc(256);

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);

	strcpy(buf, keyboard_3ds_get(255, file->name, "Enter name"));
	strcat(newPath, buf);
	free(buf);

	if (file->isDir)
	{
		if (R_FAILED(ret = FS_RenameDir(archive, oldPath, newPath)))
			return ret;
	}
	else
	{
		if (R_FAILED(ret = FS_RenameFile(archive, oldPath, newPath)))
			return ret;
	}

	Dirlist_PopulateFiles(true);
	return 0;
}

static Result FileOptions_Delete(void)
{
	File * file = Dirlist_GetFileIndex(position);

	if (file == NULL)
		return -1;
	
	if (strncmp(file->name, "..", 2) == 0)
			return -2;
	
	/*if ((recycleBin) && !(strstr(cwd, "/3ds/3DShell/bin/") != NULL))
	{
		isMovingToBin = true;
		char * buf = (char *)malloc(1024);
		copy(COPY_DELETE_ON_FINISH);
		strcpy(buf, cwd);
		memset(cwd, 0, sizeof(cwd));
		sprintf(cwd, "/3ds/3DShell/bin/");
		paste();
		strcpy(cwd, buf);
		free(buf);
		return 0;
	}*/	

	char path[1024]; // File path

	// Puzzle path
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	Result ret = 0;

	if (file->isDir) // Delete folder
	{
		if (R_FAILED(ret = FS_RmdirRecursive(archive, path)))
			return ret;
	}
	else // Delete file
	{
		if (R_FAILED(ret = FS_Remove(archive, path)))
			return ret;
	}
	
	return 0;
}

void Menu_DisplayFileOptions(void)
{
	pp2d_draw_texture(TEXTURE_OPTIONS, 37, 20);

	pp2d_draw_rectangle(37 + (selectionX * 123), 56 + (selectionY * 37), 123, 37, RGBA8(Options_select_colour.r, Options_select_colour.g, Options_select_colour.b, 255));

	pp2d_draw_text(42, 36, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][0]);
	pp2d_draw_text(232, 196, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][8]);

	pp2d_draw_text(47, 72, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][1]);
	pp2d_draw_text(47, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][3]);
	pp2d_draw_text(47, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][5]);

	pp2d_draw_text(170, 72, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][2]);

	if (copyF)
	{
		pp2d_draw_text(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
		pp2d_draw_text(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][6]);
	}
	else if (cutF)
	{
		pp2d_draw_text(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
		pp2d_draw_text(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][4]);
	}
	else
	{
		pp2d_draw_text(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][4]);
		pp2d_draw_text(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][6]);
	}
}

void Menu_ControlFileOptions(u32 input)
{
	if ((input & KEY_TOUCH) && (touchInRect(37, 179, 282, 217))) // Cancel
	{
		wait(1);
		copyF = false;
		cutF = false;
		MENU_DEFAULT_STATE = MENU_STATE_HOME;
	}

	else if ((input & KEY_TOUCH) && (touchInRect(37, 56, 160, 93)))
	{
		selectionX = 0;
		selectionY = 0;
		wait(1);

		MENU_DEFAULT_STATE = MENU_STATE_PROPERTIES;
	}

	else if ((input & KEY_TOUCH) && (touchInRect(37, 94, 160, 130)))
	{
		selectionX = 0;
		selectionY = 1;
		wait(1);

		FileOptions_CreateFolder();
	}

	else if ((input & KEY_TOUCH) && (touchInRect(161, 56, 284, 93)))
	{
		selectionX = 1;
		selectionY = 0;
		wait(1);

		FileOptions_RenameFile();
	}
	
	/*else if ((input & KEY_TOUCH) && (touchInRect(37, 131, 160, 167)))
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

	if ((CAN_COPY) && (input & KEY_TOUCH) && (touchInRect(161, 94, 284, 130)))
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
	}
	
	else if ((copyF) && (input & KEY_TOUCH) && (touchInRect(161, 94, 284, 130)))
	{
		selectionX = 0;
		selectionY = 0;
		wait(1);

		if (paste() == 0)
		{
			copyFromNand = false;
			copyFromSD = false;
			copyF = false;
			Dirlist_PopulateFiles(true);
			Dirlist_DisplayFiles();
		}
	}

	if ((CAN_CUT) && (input & KEY_TOUCH) && (touchInRect(161, 131, 284, 167)))
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
		}
	}
			
	else if ((cutF) && (input & KEY_TOUCH) && (touchInRect(161, 131, 284, 167)))
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
				Dirlist_PopulateFiles(true);
				Dirlist_DisplayFiles();
			}
		}
	}*/
}