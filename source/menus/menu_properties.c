#include "common.h"
#include "dir_list.h"
#include "language.h"
#include "menu_properties.h"
#include "pp2d.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"
#include "utils.h"

struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Options_title_text_colour;
struct colour Settings_title_text_colour;

void Menu_DisplayProperties(void)
{
	File * file = Dirlist_GetFileIndex(position);

	char path[255], fullPath[500];

	strcpy(fileName, file->name);

	strcpy(path, cwd);

	char fileSize[16];
	if (!(file->isDir))
		Utils_GetSizeString(fileSize, file->size);

	u32 totalSize = 0, directories = 0, files = 0;

		pp2d_draw_texture(TEXTURE_PROPERTIES, 36, 20);

		pp2d_draw_text(41, 33, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_properties[language][0]);

		pp2d_draw_text(247, 201, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_properties[language][6]);

		pp2d_draw_text(((320 - pp2d_get_text_width(lang_properties[language][1], 0.45f, 0.45f)) / 2), 50, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][1]);

		pp2d_draw_text(42, 74, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][2]);
			pp2d_draw_textf(100, 74, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%.28s", fileName);
		pp2d_draw_text(42, 94, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][3]);
			pp2d_draw_textf(100, 94, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%.28s", path);

		pp2d_draw_text(42, 114, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][4]);

		if (file->isDir)
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][1]);
		else if ((strncmp(file->ext, "CIA", 3) == 0) || (strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "3DS", 3) == 0) || (strncmp(file->ext, "3ds", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][2]);
		else if ((strncmp(file->ext, "bin", 3) == 0) || (strncmp(file->ext, "BIN", 3) == 0) || (strncmp(file->ext, "fir", 3) == 0) || (strncmp(file->ext, "FIR", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][3]);
		else if ((strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][4]);
		else if ((strncmp(file->ext, "rar", 3) == 0) || (strncmp(file->ext, "RAR", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][5]);
		else if ((strncmp(file->ext, "PNG", 3) == 0) || (strncmp(file->ext, "png", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][6]);
		else if ((strncmp(file->ext, "JPG", 3) == 0) || (strncmp(file->ext, "jpg", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][7]);
		else if ((strncmp(file->ext, "MP3", 3) == 0) || (strncmp(file->ext, "mp3", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][8]);
		else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0) || (strncmp(file->ext, "XML", 3) == 0) || (strncmp(file->ext, "xml", 3) == 0))
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][9]);
		else
			pp2d_draw_text(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][10]);

		if (!(file->isDir))
		{
			pp2d_draw_text(42, 134, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][5]);
			pp2d_draw_textf(100, 134, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%.28s", fileSize);
		}
}

void Menu_ControlProperties(u32 input)
{
	if ((input & KEY_B) || (input & KEY_A) || (touchInRect(36, 192, 284, 220)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
	}
}