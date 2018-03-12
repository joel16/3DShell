#ifndef CLOCK_H
#define CLOCK_H

#include <3ds.h>

char * Clock_GetCurrentTime(bool _12hour);
char * Clock_GetDayOfWeek(bool brief);
char * Clock_GetMonthOfYear(int style);

#endif