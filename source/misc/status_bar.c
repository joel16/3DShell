#include "battery.h"
#include "clock.h"
#include "pp2d.h"
#include "status_bar.h"
#include "textures.h"
#include "wifi.h"

void StatusBar_DisplayBar(void)
{
	float time_x = 395 - pp2d_get_text_width(Clock_GetCurrentTime(true), 0.45f, 0.45f);
	float battery_texture_x = time_x - pp2d_get_texture_width(TEXTURE_BATTERY_100) - 5;
	float battery_string_x = battery_texture_x - pp2d_get_text_width(Battery_GetBatteryPercentString(), 0.45f, 0.45f) - 5;
	float wifi_x = battery_string_x - pp2d_get_texture_width(TEXTURE_WIFI_3) - 10;

	pp2d_draw_text(time_x, 1, 0.45f, 0.45f, RGBA8(255, 255, 255, 255), Clock_GetCurrentTime(true));

	Battery_DrawIcons(battery_texture_x, 1);

	pp2d_draw_text(battery_string_x, 1, 0.45f, 0.45f, RGBA8(255, 255, 255, 255), Battery_GetBatteryPercentString());

	WiFi_DrawIcons(wifi_x, 1);
}