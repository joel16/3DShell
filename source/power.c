#include "common.h"
#include "mcu.h"
#include "power.h"
#include "screen.h"
#include "theme.h"

struct colour TopScreen_bar_colour;

void drawBatteryStatus(int x, int y)
{
    u8 batteryPercent;
	mcuGetBatteryLevel(&batteryPercent);
	
	if(batteryPercent == 0)
		screen_draw_texture(TEXTURE_BATTERY_0, x, y);	
	else if(batteryPercent > 0 && batteryPercent <= 15)
		screen_draw_texture(TEXTURE_BATTERY_15, x, y);
	else if(batteryPercent > 15 && batteryPercent <= 28)
		screen_draw_texture(TEXTURE_BATTERY_28, x, y);
	else if(batteryPercent > 28 && batteryPercent <= 43)
		screen_draw_texture(TEXTURE_BATTERY_43, x, y);
	else if(batteryPercent > 43 && batteryPercent <= 57)
		screen_draw_texture(TEXTURE_BATTERY_57, x, y);
	else if(batteryPercent > 57 && batteryPercent <= 71)
		screen_draw_texture(TEXTURE_BATTERY_71, x, y);
	else if(batteryPercent > 71 && batteryPercent <= 99)
		screen_draw_texture(TEXTURE_BATTERY_85, x, y);
	else if(batteryPercent == 100)
		screen_draw_texture(TEXTURE_BATTERY_100, x, y);
	
	u8 batteryState; // boolean that represnets charging state
	PTMU_GetBatteryChargeState(&batteryState);
	
	if (batteryState == 1)
		screen_draw_texture(TEXTURE_BATTERY_CHARGE, x, y);
	
	if(batteryPercent == 100)
		screen_draw_stringf(x + 15, y - 1, 0.41f, 0.41f, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), "100%%");
	else
		screen_draw_stringf(x + 20, y - 1, 0.41f, 0.41f, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), "%2d%%", batteryPercent);
}