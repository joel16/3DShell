#include "keyboard.h"

char * keyboard_3ds_get(int maxTextLength, const char* hintText)
{
	static SwkbdState swkbd;
	
	char * str = malloc(maxTextLength);
	memset(str, 0, maxTextLength);
	
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, maxTextLength);
    swkbdSetHintText(&swkbd, hintText);
	swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
	swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Confirm", true);
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_HOME);
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_RESET);
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_POWER);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdInputText(&swkbd, str, maxTextLength);
	
	return str;
}