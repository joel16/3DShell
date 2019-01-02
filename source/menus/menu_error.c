#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "C2D_helper.h"
#include "config.h"
#include "textures.h"
#include "touch.h"

void Menu_DisplayError(char *msg, Result ret) {
	float msg_width = 0, result_width = 0, confirm_width = 0, confirm_height = 0;
	
	char *result = malloc(64);
	if (ret != 0)
		snprintf(result, 64, "Ret: 0x%lx\n", ret);

	Draw_GetTextSize(0.45f, &msg_width, NULL, msg);
	Draw_GetTextSize(0.45f, &result_width, NULL, result);
	Draw_GetTextSize(0.45f, &confirm_width, &confirm_height, "OK");

	while(aptMainLoop()) {
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar

		Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

		Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Error");

		Draw_Text(((320 - (msg_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 34, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, msg);
		
		if (ret != 0)
			Draw_Text(((320 - (result_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 48, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, result);

		Draw_Rect((288 - confirm_width) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
		Draw_Text(288 - confirm_width, (159 - confirm_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");

		Draw_EndFrame();

		hidScanInput();
		u32 kDown = hidKeysDown();
		if ((kDown & KEY_A) || (kDown & KEY_B))
			break;

		if (TouchInRect((288 - confirm_width) - 5, (159 - confirm_height) - 5, ((288 - confirm_width) - 5) + confirm_width + 10, ((159 - confirm_height) - 5) + confirm_height + 10)) {
			if (kDown & KEY_TOUCH)
				break;
		}
	}

	free(result);
}
