#include <3ds.h>
#include <time.h>

#include "C2D_helper.h"
#include "common.h"
#include "status_bar.h"

static char *Clock_GetCurrentTime(bool _12hour)
{
	static char buffer[10];

	time_t unix_time = time(0);
	struct tm* time_struct = gmtime((const time_t*)&unix_time);
	int hours = time_struct->tm_hour;
	int minutes = time_struct->tm_min;
	int amOrPm = 0;

	if (_12hour)
	{
		if (hours < 12)
			amOrPm = 1;
		if (hours == 0)
			hours = 12;
		else if (hours > 12)
			hours = hours - 12;

		if ((hours >= 1) && (hours < 10))
			snprintf(buffer, 10, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
		else
			snprintf(buffer, 10, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
	}

	return buffer;
}

void StatusBar_DisplayTime(void)
{
	float width = 0, height = 0;
	Draw_GetTextSize(0.48f, &width, &height, Clock_GetCurrentTime(true));

	Draw_Text(390 - width, (18 - height) / 2, 0.48f, WHITE, Clock_GetCurrentTime(true));
}