#include <3ds.h>
#include <time.h>

#include "C2D_helper.h"
#include "common.h"
#include "status_bar.h"
#include "textures.h"

static float percent_width = 0.0f;

static char *Clock_GetCurrentTime(bool _12hour) {
	static char buffer[27];

	time_t unix_time = time(0);
	struct tm* time_struct = gmtime((const time_t*)&unix_time);
	int hours = time_struct->tm_hour;
	int minutes = time_struct->tm_min;
	int amOrPm = 0;

	if (_12hour) {
		if (hours < 12)
			amOrPm = 1;
		if (hours == 0)
			hours = 12;
		else if (hours > 12)
			hours = hours - 12;

		if ((hours >= 1) && (hours < 10))
			snprintf(buffer, 27, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
		else
			snprintf(buffer, 27, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
	}

	return buffer;
}

static void StatusBar_GetBatteryStatus(int x, int y) {
	u8 percent = 0, state = 0;
	char buf[5];

	if (R_FAILED(PTMU_GetBatteryChargeState(&state)))
		state = 0;

	if (R_SUCCEEDED(MCUHWC_GetBatteryLevel(&percent))) {
		if (percent < 20)
			Draw_Image(battery_low, x, 1);
		else if ((percent >= 20) && (percent < 30)) {
			if (state == 1)
				Draw_Image(battery_20_charging, x, 1);
			else
				Draw_Image(battery_20, x, 1);
		}
		else if ((percent >= 30) && (percent < 50)) {
			if (state == 1)
				Draw_Image(battery_50_charging, x, 1);
			else
				Draw_Image(battery_50, x, 1);
		}
		else if ((percent >= 50) && (percent < 60)) {
			if (state == 1)
				Draw_Image(battery_50_charging, x, 1);
			else
				Draw_Image(battery_50, x, 1);
		}
		else if ((percent >= 60) && (percent < 80)) {
			if (state == 1)
				Draw_Image(battery_60_charging, x, 1);
			else
				Draw_Image(battery_60, x, 1);
		}
		else if ((percent >= 80) && (percent < 90)) {
			if (state == 1)
				Draw_Image(battery_80_charging, x, 1);
			else
				Draw_Image(battery_80, x, 1);
		}
		else if ((percent >= 90) && (percent < 100)) {
			if (state == 1)
				Draw_Image(battery_90_charging, x, 1);
			else
				Draw_Image(battery_90, x, 1);
		}
		else if (percent == 100) {
			if (state == 1)
				Draw_Image(battery_full_charging, x, 1);
			else
				Draw_Image(battery_full, x, 1);
		}

		snprintf(buf, 5, "%d%%", percent);
		percent_width = Draw_GetTextWidth(0.45f, buf);
		Draw_Text((float)(x - percent_width - 5), y, 0.45f, WHITE, buf);
	}
	else {
		snprintf(buf, 5, "%d%%", percent);
		percent_width = Draw_GetTextWidth(0.45f, buf);
		Draw_Text((float)(x - percent_width - 5), y, 0.45f, WHITE, buf);
		Draw_Image(battery_unknown, x, 1);
	}
}

static void StatusBar_GetWifiStatus(int x) {
	switch(osGetWifiStrength()) {
		case 0:
			Draw_Image(icon_wifi_0, x, 2);
			break;
		case 1:
			Draw_Image(icon_wifi_1, x, 2);
			break;
		case 2:
			Draw_Image(icon_wifi_2, x, 2);
			break;
		case 3:
			Draw_Image(icon_wifi_3, x, 2);
			break;
	}
}

void StatusBar_DisplayTime(void) {
	float width = 0, height = 0;
	Draw_GetTextSize(0.45f, &width, &height, Clock_GetCurrentTime(true));

	StatusBar_GetBatteryStatus((float)((390 - width) - (10 + 12)), (float)((18 - height) / 2));
	StatusBar_GetWifiStatus((float)((390 - width) - (10 + 14) - (10 + 12) - (percent_width + 10)));

	Draw_Text((float)(390 - width), (float)((18 - height) / 2), 0.45f, WHITE, Clock_GetCurrentTime(true));
}
