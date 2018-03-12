#include <stdio.h>
#include <time.h>

#include "clock.h"
#include "language.h"

char * Clock_GetCurrentTime(bool _12hour)
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

char * Clock_GetDayOfWeek(bool brief)
{
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);

	static char buffer[16];
	sprintf(buffer, "%s", lang_days[language][timeStruct->tm_wday]);

    if (brief)
        buffer[3] = 0;

    return buffer;
}

char * Clock_GetMonthOfYear(int style)
{
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	int day = timeStruct->tm_mday;
	int year = timeStruct->tm_year + 1900;

	static char buffer[16];

	if (style == 0)
		sprintf(buffer, "%d %s, %d", day, lang_months[language][timeStruct->tm_mon], year);
	else
		sprintf(buffer, "%s", lang_months[language][timeStruct->tm_mon]);

	if (style == 1)
		buffer[3] = 0;

	return buffer;
}
