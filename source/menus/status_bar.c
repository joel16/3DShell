#include <3ds.h>

#include <stdio.h>
#include <time.h>

#include "common.h"
#include "graphics/screen.h"
#include "language.h"
#include "main.h"
#include "menus/menu_music.h"
#include "menus/status_bar.h"
#include "theme.h"

struct colour TopScreen_bar_colour;

void drawBatteryStatus(void)
{
	u8 batteryPercent = 0;

	if (R_SUCCEEDED(mcuHwcGetBatteryLevel(&batteryPercent)))
	{
		if (batteryPercent == 0)
			screen_draw_texture(TEXTURE_BATTERY_0, 280, 1);
		else if (batteryPercent > 0 && batteryPercent <= 15)
			screen_draw_texture(TEXTURE_BATTERY_15, 280, 1);
		else if (batteryPercent > 15 && batteryPercent <= 28)
			screen_draw_texture(TEXTURE_BATTERY_28, 280, 1);
		else if (batteryPercent > 28 && batteryPercent <= 43)
			screen_draw_texture(TEXTURE_BATTERY_43, 280, 1);
		else if (batteryPercent > 43 && batteryPercent <= 57)
			screen_draw_texture(TEXTURE_BATTERY_57, 280, 1);
		else if (batteryPercent > 57 && batteryPercent <= 71)
			screen_draw_texture(TEXTURE_BATTERY_71, 280, 1);
		else if (batteryPercent > 71 && batteryPercent <= 99)
			screen_draw_texture(TEXTURE_BATTERY_85, 280, 1);
		else if (batteryPercent == 100)
			screen_draw_texture(TEXTURE_BATTERY_100, 280, 1);
	}

	u8 batteryState = false; // boolean that represnets charging state

	if (R_SUCCEEDED(PTMU_GetBatteryChargeState(&batteryState)))
	{
		if (batteryState)
			screen_draw_texture(TEXTURE_BATTERY_CHARGE, 282, 3);
	}
		
	screen_draw_stringf(300, 2, 0.44f, 0.44f, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), "%2d%%", batteryPercent);
}

void digitalTime(void)
{
	float width = 0;

	time_t unix_time = time(0);
	struct tm* time_struct = gmtime((const time_t*)&unix_time);
	int hours = time_struct->tm_hour;
	int minutes = time_struct->tm_min;
	int amOrPm = 0;

	if (hours < 12)
		amOrPm = 1;
	if (hours == 0)
		hours = 12;
	else if (hours > 12)
		hours = hours - 12;

	if ((hours >= 1) && (hours < 10))
	{
		width = screen_get_string_width("0:00 XM", 0.44f, 0.44f);
		screen_draw_stringf((393 - width), 2, 0.44f, 0.44f, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
	}
	else
	{
		width = screen_get_string_width("00:00 XM", 0.44f, 0.44f);
		screen_draw_stringf((393 - width), 2, 0.44f, 0.44f, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
	}
}

char * getDayOfWeek(int type)
{
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);

	static char buffer[16];
	sprintf(buffer, "%s", lang_days[language][timeStruct->tm_wday]);

    if (type == 1)
        buffer[3] = 0;

    return buffer;
}

char * getMonthOfYear(int type)
{
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	int day = timeStruct->tm_mday;
	int year = timeStruct->tm_year + 1900;

	static char buffer[16];

	if (type == 0)
		sprintf(buffer, "%d %s, %d", day, lang_months[language][timeStruct->tm_mon], year);
	else
		sprintf(buffer, "%s", lang_months[language][timeStruct->tm_mon]);

	if (type == 1)
		buffer[3] = 0;

	return buffer;
}

void drawWifiStatus(void)
{
	switch (osGetWifiStrength())
	{
		case 0:
			screen_draw_texture(TEXTURE_WIFI_0, 255, 1);
			break;
		
		case 1:
			screen_draw_texture(TEXTURE_WIFI_1, 255, 1);
			break;
		
		case 2:
			screen_draw_texture(TEXTURE_WIFI_2, 255, 1);
			break;
		
		case 3:
			screen_draw_texture(TEXTURE_WIFI_3, 255, 1);
			break;
		
		default:
			screen_draw_texture(TEXTURE_WIFI_NULL, 255, 1);
	}	
}

void drawStatusBar(void)
{
	if (isPlaying())
		screen_draw_texture(TEXTURE_MUSIC_STATUS, 4, 1);

	drawWifiStatus();
	drawBatteryStatus();
	digitalTime();
}