#include "common.h"
#include "graphics/screen.h"
#include "wifi.h"

void drawWifiStatus(int x, int y)
{
	if (osGetWifiStrength() == 0)
		screen_draw_texture(TEXTURE_WIFI_0, x, y);
	else if (osGetWifiStrength() == 1)
		screen_draw_texture(TEXTURE_WIFI_1, x, y);
	else if (osGetWifiStrength() == 2)
		screen_draw_texture(TEXTURE_WIFI_2, x, y);
	else if (osGetWifiStrength() == 3)
		screen_draw_texture(TEXTURE_WIFI_3, x, y);
	else
		screen_draw_texture(TEXTURE_WIFI_NULL, x, y);
}
