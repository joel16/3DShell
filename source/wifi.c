#include "common.h"
#include "graphics/screen.h"
#include "wifi.h"

void drawWifiStatus(void)
{
	if (osGetWifiStrength() == 0)
		screen_draw_texture(TEXTURE_WIFI_0, 260, 2);
	else if (osGetWifiStrength() == 1)
		screen_draw_texture(TEXTURE_WIFI_1, 260, 2);
	else if (osGetWifiStrength() == 2)
		screen_draw_texture(TEXTURE_WIFI_2, 260, 2);
	else if (osGetWifiStrength() == 3)
		screen_draw_texture(TEXTURE_WIFI_3, 260, 2);
	else
		screen_draw_texture(TEXTURE_WIFI_NULL, 260, 2);
}
