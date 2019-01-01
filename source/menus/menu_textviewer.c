#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "menu_main.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

void Menu_DisplayText(char *path) {
	// TODO: use a more efficent way to do this.
	char line[512][59]; // 512 lines, with length 59
	int line_count = 0, selection = 9;
	char activity[59];
	int position = 0;

	FILE *fp = fopen(path, "rb");

	while (fgets(line[line_count], 59, fp)) {
		sscanf(line[line_count], "%d\t%[^\n]", &position, activity);
		//line[line_count][strcspn(line[line_count], "\n")] = '\0';
		line_count++;
	}
	
	fclose(fp);

    float height = 0, title_height = 0;
    Draw_GetTextSize(0.48f, NULL, &height, "QWERTY");
    Draw_GetTextSize(0.48f, NULL, &title_height, cwd);

    while(aptMainLoop()) {
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, config.dark_theme? BLACK_BG : WHITE);
		C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_TOP);

		Draw_Rect(0, 0, 400, 18, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT); // Status bar
		Draw_Rect(0, 18, 400, 34, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT); // Menu bar

		StatusBar_DisplayTime();
		Draw_Text(10, 18 + ((34 - title_height) / 2), 0.48f, WHITE, Utils_Basename(path));

		int printed = 0; // Print counter

		for (int index = 0; index < line_count; index++) {
			if (printed == 10)
				break;

			if (selection < 10 || index > (selection - 10)) {
				Draw_Textf(5, 56 + ((18 - height) / 2) + (18 * printed), 0.48f, config.dark_theme? WHITE : BLACK, "%d)  %s", index, line[index]);
				printed++;
			}
		}

		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar

		Menu_DrawMenuBar();

		Draw_EndFrame();

		hidScanInput();
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		if (line_count > 9) {
			if (kDown & KEY_DDOWN)
				selection++;
			else if (kDown & KEY_DUP)
				selection--;

			if (kHeld & KEY_CPAD_UP) {
				wait(5);
				selection--;
			}
			else if (kHeld & KEY_CPAD_DOWN) {
				wait(5);
				selection++;
			}
		}

		Utils_SetMax(&selection, 9, line_count - 1);
		Utils_SetMin(&selection, line_count - 1, 9);

		if (kDown & KEY_B)
			break;
	}
}
