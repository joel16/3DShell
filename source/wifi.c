#include "common.h"
#include "graphics/screen.h"
#include "wifi.h"

void drawWifiStatus(void)
{
	if (osGetWifiStrength() == 0)
		screen_draw_texture(TEXTURE_WIFI_0, 255, 1);
	else if (osGetWifiStrength() == 1)
		screen_draw_texture(TEXTURE_WIFI_1, 255, 1);
	else if (osGetWifiStrength() == 2)
		screen_draw_texture(TEXTURE_WIFI_2, 255, 1);
	else if (osGetWifiStrength() == 3)
		screen_draw_texture(TEXTURE_WIFI_3, 255, 1);
	else
		screen_draw_texture(TEXTURE_WIFI_NULL, 255, 1);
}
