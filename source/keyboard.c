#include <3ds.h>
#include <stdlib.h>
#include <string.h>

#include "C2D_helper.h"
#include "keyboard.h"

const char *Keyboard_GetText(const char *initial_text, const char *hint_text) {
	Draw_EndFrame();

	static SwkbdState swkbd;
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	static bool reload = false;
	static char input_string[256];

	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 256);

	if (strlen(hint_text) != 0)
		swkbdSetHintText(&swkbd, hint_text);
	
	if (strlen(initial_text) != 0)
		swkbdSetInitialText(&swkbd, initial_text);

	swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
	swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Confirm", true);

	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_HOME);
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_RESET);
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_POWER);
	swkbdSetFeatures(&swkbd, SWKBD_PREDICTIVE_INPUT);

	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);

	SwkbdDictWord words[7];
	swkbdSetDictWord(&words[0], ".3dsx", ".3dsx");
	swkbdSetDictWord(&words[1], ".cia", ".cia");
	swkbdSetDictWord(&words[2], "github", "github");
	swkbdSetDictWord(&words[3], "http://", "http://");
	swkbdSetDictWord(&words[4], "https://", "https://");
	swkbdSetDictWord(&words[5], "releases", "releases");
	swkbdSetDictWord(&words[6], "/3ds/", "/3ds/");
	swkbdSetDictionary(&swkbd, words, 7);

	swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
	swkbdSetLearningData(&swkbd, &swkbdLearning, reload, true);
	reload = true;

	swkbdInputText(&swkbd, input_string, 256);

	return input_string;
}
