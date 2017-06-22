#include "keyboard.h"

char * keyboard_3ds_get(int maxTextLength, const char* initialText, const char* hintText)
{
	static SwkbdState swkbd;
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	
	char * str = malloc(maxTextLength);
	memset(str, 0, maxTextLength);
	
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, maxTextLength);
	
    swkbdSetHintText(&swkbd, hintText);
	
	swkbdSetInitialText(&swkbd, initialText);
	
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
	swkbdSetDictionary(&swkbd, words, sizeof(words)/sizeof(SwkbdDictWord));
	static bool reload = false;
	swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
	swkbdSetLearningData(&swkbd, &swkbdLearning, reload, true);
	reload = true;
	
	swkbdInputText(&swkbd, str, maxTextLength);
	
	return str;
}