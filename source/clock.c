#include "clock.h"
#include "main.h"
#include "theme.h"

struct colour TopScreen_bar_colour;

void digitalTime()
{
	time_t unix_time = time(0);
	struct tm* time_struct = gmtime((const time_t*)&unix_time);
	int hours = time_struct->tm_hour;
	int minutes = time_struct->tm_min;
	int amOrPm = 0;
	
	if(hours < 12)
		amOrPm = 1;
	if(hours == 0)
		hours = 12;
	else if(hours > 12)
		hours = hours - 12;

	if (hours >= 1 && hours < 10)  
		sftd_draw_textf(font, 395 - sftd_get_text_width(font, 11, "0:00 XM"), 1, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), 11, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
	else
		sftd_draw_textf(font, 395 - sftd_get_text_width(font, 11, "00:00 XM"), 1, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), 11, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
}

char * getDayOfWeek(int type)
{
	static const char days[7][16] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	
	static char buffer[16];
	sprintf(buffer, "%s", days[timeStruct->tm_wday]);
    
    if(type == 1)
        buffer[3] = 0;
	
    return buffer;
}

char * getMonthOfYear(int type)
{
	static const char months[12][16] =
	{
		"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
	};
	
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	int day = timeStruct->tm_mday;
	int year = timeStruct->tm_year + 1900;
	
	static char buffer[16];
	
	if (type == 0)
		sprintf(buffer, "%d %s, %d", day, months[timeStruct->tm_mon], year);
	else
		sprintf(buffer, "%s", months[timeStruct->tm_mon]);
	
	if (type == 1)
		buffer[3] = 0;
	
	return buffer;
}