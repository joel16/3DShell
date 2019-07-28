#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "C2D_helper.h"
#include "config.h"
#include "dialog.h"
#include "touch.h"

void Menu_DisplayError(const char *msg, Result ret) {
	float confirm_width = 0, confirm_height = 0;
	
	char *result = malloc(64);
	if (ret != 0)
		snprintf(result, 64, "Ret: 0x%lx\n", ret);
	
	Draw_GetTextSize(0.42f, &confirm_width, &confirm_height, "OK");

	while(aptMainLoop()) {
		Dialog_DisplayMessage("Error", msg, result, true);

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
