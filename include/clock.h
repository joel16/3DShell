#ifndef CLOCK_H
#define CLOCK_H

#include <3ds.h>
#include <time.h>
#include <sftd.h>

void digitalTime();
char * getDayOfWeek(int type);
char * getMonthOfYear(int type);

#endif