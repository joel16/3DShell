#include "common.h"
#include "dir_list.h"
#include "language.h"
#include "menu_sort.h"
#include "pp2d.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"
#include "utils.h"

struct colour Settings_colour;
struct colour Settings_title_text_colour;
struct colour Settings_text_colour;
struct colour Settings_text_min_colour;

void Menu_DisplaySort(void)
{
	pp2d_draw_rectangle(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));

	pp2d_draw_text(10, 30, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "Sorting options");

	pp2d_draw_text(10, 50, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "None");
	pp2d_draw_text(10, 62, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "No sorting method is applied.");

	pp2d_draw_text(10, 85, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Alphabetical \uE01B");
	pp2d_draw_text(10, 97, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort alphabetically in ascending order.");

	pp2d_draw_text(10, 120, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Alphabetical \uE01C");
	pp2d_draw_text(10, 132, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort alphabetically in descending order.");

	pp2d_draw_text(10, 155, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Size \uE01B");
	pp2d_draw_text(10, 167, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort by size (largest first).");

	pp2d_draw_text(10, 190, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Size \uE01C");
	pp2d_draw_text(10, 202, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Sort by size (smallest first).");

	pp2d_draw_texture(sortBy == 0? TEXTURE_RADIO_ON : TEXTURE_RADIO_OFF, 280, 55);
	pp2d_draw_texture(sortBy == 1? TEXTURE_RADIO_ON : TEXTURE_RADIO_OFF, 280, 90);
	pp2d_draw_texture(sortBy == 2? TEXTURE_RADIO_ON : TEXTURE_RADIO_OFF, 280, 125);
	pp2d_draw_texture(sortBy == 3? TEXTURE_RADIO_ON : TEXTURE_RADIO_OFF, 280, 160);
	pp2d_draw_texture(sortBy == 4? TEXTURE_RADIO_ON : TEXTURE_RADIO_OFF, 280, 195);
}

void Menu_ControlSort(u32 input)
{
	if ((input & KEY_TOUCH) && (touchInRect(280, 55, 320, 75)))
	{
		wait(1);
		sortBy = 0;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}

	else if ((input & KEY_TOUCH) && (touchInRect(280, 90, 320, 110)))
	{
		wait(1);
		sortBy = 1;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}

	else if ((input & KEY_TOUCH) && (touchInRect(280, 125, 320, 145)))
	{
		wait(1);
		sortBy = 2;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}

	else if ((input & KEY_TOUCH) && (touchInRect(280, 160, 320, 180)))
	{
		wait(1);
		sortBy = 3;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}

	else if ((input & KEY_TOUCH) && (touchInRect(280, 195, 320, 215)))
	{
		wait(1);
		sortBy = 4;
		Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
		Dirlist_PopulateFiles(true);
		Dirlist_DisplayFiles();
	}

	if (input & KEY_B)
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_SETTINGS;
	}
}