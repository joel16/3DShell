#include "main.h"
#include "mcu.h"
#include "power.h"

void drawBatteryStatus(int x, int y)
{
    u8 batteryPercent;
	mcuGetBatteryLevel(&batteryPercent);
	
	if(batteryPercent == 0)
		sf2d_draw_texture(_0, x, y);	
	else if(batteryPercent > 0 && batteryPercent <= 15)
		sf2d_draw_texture(_15, x, y);
	else if(batteryPercent > 15 && batteryPercent <= 28)
		sf2d_draw_texture(_28, x, y);
	else if(batteryPercent > 28 && batteryPercent <= 43)
		sf2d_draw_texture(_43, x, y);
	else if(batteryPercent > 43 && batteryPercent <= 57)
		sf2d_draw_texture(_57, x, y);
	else if(batteryPercent > 57 && batteryPercent <= 71)
		sf2d_draw_texture(_71, x, y);
	else if(batteryPercent > 71 && batteryPercent <= 99)
		sf2d_draw_texture(_85, x, y);
	else if(batteryPercent == 100)
		sf2d_draw_texture(_100, x, y);
	
	if(batteryPercent == 100)
		sftd_draw_textf(font, x + 15, y - 1, RGBA8(255, 255, 255, 255), 11, "100%%");
	else
		sftd_draw_textf(font, x + 20, y - 1, RGBA8(255, 255, 255, 255), 11, "%3d%%", batteryPercent);
}