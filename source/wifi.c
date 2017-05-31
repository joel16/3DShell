#include "wifi.h"

void drawWifiStatus(int x, int y)
{
	if (osGetWifiStrength() == 0)
		sf2d_draw_texture(wifiIcon0, x, y);
	else if (osGetWifiStrength() == 1)
		sf2d_draw_texture(wifiIcon1, x, y);
	else if (osGetWifiStrength() == 2)
		sf2d_draw_texture(wifiIcon2, x, y);
	else if (osGetWifiStrength() == 3)
		sf2d_draw_texture(wifiIcon3, x, y);
	else 
		sf2d_draw_texture(wifiIconNull, x, y);
}