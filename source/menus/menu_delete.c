#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "language.h"
#include "menu_delete.h"
#include "pp2d.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"
#include "utils.h"


struct colour BottomScreen_colour;
struct colour Options_title_text_colour;
struct colour Settings_title_text_colour;

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

void Menu_DisplayDeleteDialog(void)
{
	pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));

	pp2d_draw_texture(TEXTURE_DIALOG, 20, 55);

	pp2d_draw_text(27, 72, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][0]);

	pp2d_draw_text(206, 159, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][3]);
	pp2d_draw_text(255, 159, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][4]);

	pp2d_draw_text(((320 - pp2d_get_text_width(lang_deletion[language][1], 0.45f, 0.45f)) / 2), 100, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_deletion[language][1]);
	pp2d_draw_text(((320 - pp2d_get_text_width(lang_deletion[language][2], 0.45f, 0.45f)) / 2), 115, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_deletion[language][2]);
}

void Menu_ControlDeleteDialog(u32 input)
{
	if ((input & KEY_A) || (touchInRect(240, 142, 320, 185)))
	{	
		if (R_SUCCEEDED(FileOptions_Delete()))
		{
			Dirlist_PopulateFiles(true);
			Dirlist_DisplayFiles();
		}

		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
	}

	else if ((input & KEY_B) || (touchInRect(136, 142, 239, 185)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
	}
}