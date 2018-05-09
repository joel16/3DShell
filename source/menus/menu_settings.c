#include "common.h"
#include "dir_list.h"
#include "language.h"
#include "menu_settings.h"
#include "pp2d.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"
#include "utils.h"

struct colour Settings_colour;
struct colour Settings_title_text_colour;
struct colour Settings_text_colour;
struct colour Settings_text_min_colour;

void Menu_DisplaySettings(void)
{
	pp2d_draw_rectangle(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));

	pp2d_draw_text(10, 30, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_settings[language][0]);

	pp2d_draw_text(10, 50, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Sort by");
	pp2d_draw_text(10, 62, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Select from a list of sorting options.");

	pp2d_draw_text(10, 85, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][5]);
	pp2d_draw_text(10, 97, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), lang_settings[language][6]);

	pp2d_draw_text(10, 120, 0.45f, 0.45f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][1]);
	pp2d_draw_text(10, 132, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), lang_settings[language][2]);

	pp2d_draw_text(10, 155, 0.45f, 0.45f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][3]);
	pp2d_draw_textf(10, 167, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "%s %s", lang_settings[language][4], theme_dir);

	pp2d_draw_text(10, 190, 0.45f, 0.45f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][7]);
	pp2d_draw_textf(10, 202, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "%s", lang_settings[language][8]);

	if (recycleBin)
		pp2d_draw_texture(TEXTURE_TOGGLE_ON, 280, 90);
	else
		pp2d_draw_texture(TEXTURE_TOGGLE_OFF, 280, 90);

	if (galleryDisplay)
		pp2d_draw_texture(TEXTURE_TOGGLE_ON, 280, 125);
	else
		pp2d_draw_texture(TEXTURE_TOGGLE_OFF, 280, 125);

	if (isHiddenEnabled)
		pp2d_draw_texture(TEXTURE_TOGGLE_ON, 280, 195);
	else
		pp2d_draw_texture(TEXTURE_TOGGLE_OFF, 280, 195);

	pp2d_draw_texture(TEXTURE_THEMES_LAUNCHER, 283, 155);
}

void Menu_ControlSettings(u32 input)
{
	if ((input & KEY_TOUCH) && (touchInRect(0, 50, 320, 72)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_SORT;
	}

	else if ((input & KEY_TOUCH) && (touchInRect(280, 90, 320, 110)))
	{
		wait(1);
		recycleBin = !recycleBin;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
	}

	else if ((input & KEY_TOUCH) && (touchInRect(280, 125, 320, 145)))
	{
		wait(1);
		galleryDisplay = !galleryDisplay;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
	}

	else if ((input & KEY_TOUCH) && (touchInRect(283, 155, 303, 175)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_THEMES;
		strcpy(cwd, "/3ds/3DShell/themes/");
		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}

	else if ((input & KEY_TOUCH) && (touchInRect(280, 195, 320, 215)))
	{
		wait(1);
		isHiddenEnabled = !isHiddenEnabled;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
		
		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}
}