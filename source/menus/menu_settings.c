#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "menu_main.h"
#include "status_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static int selection = 0, max_items = 3;
static float confirm_width = 0, confirm_height = 0;

void Menu_DisplayAbout(void) {
	float text_width1 = 0, text_width2 = 0, text_width3 = 0, text_width4 = 0;
	Draw_GetTextSize(0.45f, &text_width1, NULL, "3D Shell vx.x.x - xxxxxxx");
	Draw_GetTextSize(0.45f, &text_width2, NULL, "Author: Joel16");
	Draw_GetTextSize(0.45f, &text_width3, NULL, "Assets: Preetisketch/CyanogenMod/LineageOS");
	Draw_GetTextSize(0.45f, &text_width4, NULL, "Music player: deltabeard");
	Draw_GetTextSize(0.45f, &confirm_width, &confirm_height, "OK");

	Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

	Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "About");

	Draw_Textf(((320 - (text_width1)) / 2), ((240 - (dialog.subtex->height)) / 2) + 20, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "3D Shell v%d.%d.%d - %s", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO, GITVERSION);
	Draw_Text(((320 - (text_width2)) / 2), ((240 - (dialog.subtex->height)) / 2) + 34, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Author: Joel16");
	Draw_Text(((320 - (text_width3)) / 2), ((240 - (dialog.subtex->height)) / 2) + 48, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Assets: Preetisketch/CyanogenMod/LineageOS");
	Draw_Text(((320 - (text_width4)) / 2), ((240 - (dialog.subtex->height)) / 2) + 62, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Music player: deltabeard");

	Draw_Rect((288 - confirm_width) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	Draw_Text(288 - confirm_width, (159 - confirm_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");
}

void Menu_ControlAbout(u32 input) {
	if ((input & KEY_B) || (input & KEY_A))
		MENU_STATE = MENU_STATE_SETTINGS;

	if (TouchInRect((288 - confirm_width) - 5, (159 - confirm_height) - 5, ((288 - confirm_width) - 5) + confirm_width + 10, ((159 - confirm_height) - 5) + confirm_height + 10))
		if (input & KEY_TOUCH)
			MENU_STATE = MENU_STATE_SETTINGS;
}

void Menu_DisplaySortSettings(void) {
	Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
	Draw_Rect(0, 20, 320, 220, config.dark_theme? BLACK_BG : WHITE);

	Menu_DrawMenuBar();

	Draw_Rect(0, 20, 400, 35, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
	Draw_Text(10, 30, 0.48f, WHITE, "Sorting options");

	Draw_Rect(0, 55 + (selection * 40), 320, 40, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	
	Draw_Text(10, 60, 0.48f, config.dark_theme? WHITE : BLACK, "Alphabetical \uE01B");
	Draw_Text(10, 75, 0.45f, config.dark_theme? WHITE : BLACK, "Sort alphabetically in ascending order.");
	Draw_Text(10, 100, 0.48f, config.dark_theme? WHITE : BLACK, "Alphabetical \uE01C");
	Draw_Text(10, 115, 0.45f, config.dark_theme? WHITE : BLACK, "Sort alphabetically in descending order.");
	Draw_Text(10, 140, 0.48f, config.dark_theme? WHITE : BLACK, "Size \uE01B");
	Draw_Text(10, 155, 0.45f, config.dark_theme? WHITE : BLACK, "Sort by size (largest first).");
	Draw_Text(10, 180, 0.48f, config.dark_theme? WHITE : BLACK, "Size \uE01C");
	Draw_Text(10, 195, 0.45f, config.dark_theme? WHITE : BLACK, "Sort by size (smallest first).");

	Draw_Image(config.sort == 0? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 60);
	Draw_Image(config.sort == 1? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 100);
	Draw_Image(config.sort == 2? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 140);
	Draw_Image(config.sort == 3? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 180);
}

void Menu_ControlSortSettings(u32 input) {
	if (input & KEY_B) {
		if (selection != 0)
			selection = 0;

		MENU_STATE = MENU_STATE_SETTINGS;
	}

	if (input & KEY_A) {
		config.sort = selection;
		Dirbrowse_PopulateFiles(true);
		Config_Save(config);
	}

	if (input & KEY_DDOWN)
		selection++;
	else if (input & KEY_DUP)
		selection--;

	if (TouchInRect(0, 55, 320, 94)) {
		selection = 0;
		
		if (input & KEY_TOUCH) {
			config.sort = selection;
			Dirbrowse_PopulateFiles(true);
			Config_Save(config);
		}
	}
	else if (TouchInRect(0, 95, 320, 134)) {
		selection = 1;

		if (input & KEY_TOUCH) {
			config.sort = selection;
			Dirbrowse_PopulateFiles(true);
			Config_Save(config);
		}
	}
	else if (TouchInRect(0, 135, 320, 174)) {
		selection = 2;
		
		if (input & KEY_TOUCH) {
			config.sort = selection;
			Dirbrowse_PopulateFiles(true);
			Config_Save(config);
		}
	}
	else if (TouchInRect(0, 175, 320, 215)) {
		selection = 3;
		
		if (input & KEY_TOUCH) {
			config.sort = selection;
			Dirbrowse_PopulateFiles(true);
			Config_Save(config);
		}
	}

	Utils_SetMax(&selection, 0, max_items);
	Utils_SetMin(&selection, max_items, 0);
}

void Menu_DisplaySettings(void) {
	Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
	Draw_Rect(0, 20, 320, 220, config.dark_theme? BLACK_BG : WHITE);

	Menu_DrawMenuBar();

	Draw_Rect(0, 20, 400, 35, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
	Draw_Text(10, 30, 0.48f, WHITE, "Settings");

	Draw_Rect(0, 55 + (selection * 40), 320, 40, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	Draw_Text(10, 60, 0.48f, config.dark_theme? WHITE : BLACK, "Sort by");
	Draw_Text(10, 75, 0.45f, config.dark_theme? WHITE : BLACK, "Select between various sorting options.");
	Draw_Text(10, 100, 0.48f, config.dark_theme? WHITE : BLACK, "Dark theme");
	Draw_Text(10, 115, 0.45f, config.dark_theme? WHITE : BLACK, "Enables dark theme mode.");
	Draw_Text(10, 140, 0.48f, config.dark_theme? WHITE : BLACK, "Hidden files");
	Draw_Text(10, 155, 0.45f, config.dark_theme? WHITE : BLACK, "Displays hidden files.");
	Draw_Text(10, 180, 0.48f, config.dark_theme? WHITE : BLACK, "About");
	Draw_Text(10, 195, 0.45f, config.dark_theme? WHITE : BLACK, "Details about application.");

	if (config.dark_theme)
		Draw_Image(config.dark_theme? icon_toggle_dark_on : icon_toggle_on, 270, 97);
	else
		Draw_Image(icon_toggle_off, 270, 97);

	Draw_Image(config.hidden_files? (config.dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 270, 137);
}

void Menu_ControlSettings(u32 input) {
	if (input & KEY_B)
		MENU_STATE = MENU_STATE_HOME;

	if (input & KEY_A) {
		switch (selection) {
			case 0:
				MENU_STATE = MENU_STATE_SORT;
				break;
			case 1:
				config.dark_theme = !config.dark_theme;
				Config_Save(config);
				break;
			case 2:
				config.hidden_files = !config.hidden_files;
				Config_Save(config);
				Dirbrowse_PopulateFiles(true);
				break;
			case 3:
				MENU_STATE = MENU_STATE_ABOUT;
				break;
		}
	}

	if (input & KEY_DDOWN)
		selection++;
	else if (input & KEY_DUP)
		selection--;

	if (TouchInRect(0, 55, 320, 94)) {
		selection = 0;

		if (input & KEY_TOUCH)
			MENU_STATE = MENU_STATE_SORT;
	}
	else if (TouchInRect(0, 95, 320, 134)) {
		selection = 1;
		
		if (input & KEY_TOUCH) {
			config.dark_theme = !config.dark_theme;
			Config_Save(config);
		}
	}
	else if (TouchInRect(0, 135, 320, 174)) {
		selection = 2;
		
		if (input & KEY_TOUCH) {
			config.hidden_files = !config.hidden_files;
			Config_Save(config);
			Dirbrowse_PopulateFiles(true);
		}
	}
	else if (TouchInRect(0, 175, 320, 215)) {
		selection = 3;
		
		if (input & KEY_TOUCH)
			MENU_STATE = MENU_STATE_ABOUT;
	}

	Utils_SetMax(&selection, 0, max_items);
	Utils_SetMin(&selection, max_items, 0);
}
