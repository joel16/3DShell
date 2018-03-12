#include <3ds.h>

#include "pp2d.h"
#include "textures.h"
#include "wifi.h"

void WiFi_DrawIcons(int x, int y)
{
	switch (osGetWifiStrength())
	{
		case 0:
			pp2d_draw_texture(TEXTURE_WIFI_0, x, y);
			break;
		
		case 1:
			pp2d_draw_texture(TEXTURE_WIFI_1, x, y);
			break;
		
		case 2:
			pp2d_draw_texture(TEXTURE_WIFI_2, x, y);
			break;
		
		case 3:
			pp2d_draw_texture(TEXTURE_WIFI_3, x, y);
			break;
		
		default:
			pp2d_draw_texture(TEXTURE_WIFI_NULL, x, y);
	}
}