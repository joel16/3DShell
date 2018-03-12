#include <3ds.h>

#include "battery.h"
#include "pp2d.h"
#include "textures.h"

static u8 Battery_GetBatteryPercent(void)
{
	u8 batteryPercent = 0;

	if (R_SUCCEEDED(MCUHWC_GetBatteryLevel(&batteryPercent)))
		return batteryPercent;

	return 0;
}

char * Battery_GetBatteryPercentString(void)
{
	static char str[5];
	snprintf(str, 5, "%d%%", Battery_GetBatteryPercent());
	return str;
}

void Battery_DrawIcons(int x, int y)
{
	u8 batteryPercent = Battery_GetBatteryPercent();
	
	if (batteryPercent == 0)
		pp2d_draw_texture(TEXTURE_BATTERY_0, x, y);
	else if (batteryPercent > 0 && batteryPercent <= 15)
		pp2d_draw_texture(TEXTURE_BATTERY_15, x, y);
	else if (batteryPercent > 15 && batteryPercent <= 28)
		pp2d_draw_texture(TEXTURE_BATTERY_28, x, y);
	else if (batteryPercent > 28 && batteryPercent <= 43)
		pp2d_draw_texture(TEXTURE_BATTERY_43, x, y);
	else if (batteryPercent > 43 && batteryPercent <= 57)
		pp2d_draw_texture(TEXTURE_BATTERY_57, x, y);
	else if (batteryPercent > 57 && batteryPercent <= 71)
		pp2d_draw_texture(TEXTURE_BATTERY_71, x, y);
	else if (batteryPercent > 71 && batteryPercent <= 99)
		pp2d_draw_texture(TEXTURE_BATTERY_85, x, y);
	else if (batteryPercent == 100)
		pp2d_draw_texture(TEXTURE_BATTERY_100, x, y);

	u8 batteryState = false; // boolean that represnets charging state

	if (R_SUCCEEDED(PTMU_GetBatteryChargeState(&batteryState)))
	{
		if (batteryState)
			pp2d_draw_texture(TEXTURE_BATTERY_CHARGE, x + 2, y + 2);
	}
}