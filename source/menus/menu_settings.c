#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "menu_main.h"
#include "menu_settings.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

static int selection = 0, max_items = 3;

void Menu_DisplaySortSettings(void)
{
	Draw_Rect(0, 0, 320, 20, config_dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
	Draw_Rect(0, 20, 320, 220, config_dark_theme? BLACK_BG : WHITE);

	Menu_DrawMenuBar();

	Draw_Rect(0, 20, 400, 35, config_dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
	Draw_Text(10, 30, 0.48f, WHITE, "Sorting options");

	Draw_Rect(0, 55 + (selection * 40), 320, 40, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	
	Draw_Text(10, 60, 0.48f, config_dark_theme? WHITE : BLACK, "Alphabetical \uE01B");
	Draw_Text(10, 75, 0.45f, config_dark_theme? WHITE : BLACK, "Sort alphabetically in ascending order.");
	Draw_Text(10, 100, 0.48f, config_dark_theme? WHITE : BLACK, "Alphabetical \uE01C");
	Draw_Text(10, 115, 0.45f, config_dark_theme? WHITE : BLACK, "Sort alphabetically in descending order.");
	Draw_Text(10, 140, 0.48f, config_dark_theme? WHITE : BLACK, "Size \uE01B");
	Draw_Text(10, 155, 0.45f, config_dark_theme? WHITE : BLACK, "Sort by size (largest first).");
	Draw_Text(10, 180, 0.48f, config_dark_theme? WHITE : BLACK, "Size \uE01C");
	Draw_Text(10, 195, 0.45f, config_dark_theme? WHITE : BLACK, "Sort by size (smallest first).");

	Draw_Image(config_sort_by == 0? (config_dark_theme? icon_radio_dark_on : icon_radio_on) : (config_dark_theme? icon_radio_dark_off : icon_radio_off), 270, 60);
	Draw_Image(config_sort_by == 1? (config_dark_theme? icon_radio_dark_on : icon_radio_on) : (config_dark_theme? icon_radio_dark_off : icon_radio_off), 270, 100);
	Draw_Image(config_sort_by == 2? (config_dark_theme? icon_radio_dark_on : icon_radio_on) : (config_dark_theme? icon_radio_dark_off : icon_radio_off), 270, 140);
	Draw_Image(config_sort_by == 3? (config_dark_theme? icon_radio_dark_on : icon_radio_on) : (config_dark_theme? icon_radio_dark_off : icon_radio_off), 270, 180);
}

void Menu_ControlSortSettings(u32 input)
{
	if (input & KEY_B)
		MENU_STATE = MENU_STATE_SETTINGS;

	if (input & KEY_A)
	{
		switch (selection)
		{
			case 0:
				config_sort_by = 0;
				break;
			case 1:
				config_sort_by = 1;
				break;
			case 2:
				config_sort_by = 2;
				break;
			case 3:
				config_sort_by = 3;
				break;
		}
		
		Dirbrowse_PopulateFiles(true);
		Config_Save(config_dark_theme, config_hidden_files, config_sort_by);
	}

	if (input & KEY_DDOWN)
		selection++;
	else if (input & KEY_DUP)
		selection--;

	Utils_SetMax(&selection, 0, max_items);
	Utils_SetMin(&selection, max_items, 0);
}

void Menu_DisplaySettings(void)
{
	Draw_Rect(0, 0, 320, 20, config_dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
	Draw_Rect(0, 20, 320, 220, config_dark_theme? BLACK_BG : WHITE);

	Menu_DrawMenuBar();

	Draw_Rect(0, 20, 400, 35, config_dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
	Draw_Text(10, 30, 0.48f, WHITE, "Settings");

	Draw_Rect(0, 55 + (selection * 40), 320, 40, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	Draw_Text(10, 60, 0.48f, config_dark_theme? WHITE : BLACK, "Sort by");
	Draw_Text(10, 75, 0.45f, config_dark_theme? WHITE : BLACK, "Select between various sorting options.");
	Draw_Text(10, 100, 0.48f, config_dark_theme? WHITE : BLACK, "Dark theme");
	Draw_Text(10, 115, 0.45f, config_dark_theme? WHITE : BLACK, "Enables dark theme mode.");
	Draw_Text(10, 140, 0.48f, config_dark_theme? WHITE : BLACK, "Hidden files");
	Draw_Text(10, 155, 0.45f, config_dark_theme? WHITE : BLACK, "Displays hidden files.");
	Draw_Text(10, 180, 0.48f, config_dark_theme? WHITE : BLACK, "About");
	Draw_Text(10, 195, 0.45f, config_dark_theme? WHITE : BLACK, "Details about application.");

	if (config_dark_theme)
		Draw_Image(config_dark_theme? icon_toggle_dark_on : icon_toggle_on, 270, 97.5);
	else
		Draw_Image(icon_toggle_off, 270, 97.5);

	Draw_Image(config_hidden_files? (config_dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 270, 137.5);
}

void Menu_ControlSettings(u32 input)
{
	if (input & KEY_B)
		MENU_STATE = MENU_STATE_HOME;

	if (input & KEY_A)
	{
		switch (selection)
		{
			case 0:
				MENU_STATE = MENU_STATE_SORT;
				break;
			case 1:
				config_dark_theme = !config_dark_theme;
				Config_Save(config_dark_theme, config_hidden_files, config_sort_by);
				break;
			case 2:
				config_hidden_files = !config_hidden_files;
				Config_Save(config_dark_theme, config_hidden_files, config_sort_by);
				break;
		}
	}

	if (input & KEY_DDOWN)
		selection++;
	else if (input & KEY_DUP)
		selection--;

	Utils_SetMax(&selection, 0, max_items);
	Utils_SetMin(&selection, max_items, 0);
}
