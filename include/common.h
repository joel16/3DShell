#ifndef COMMON_H
#define COMMON_H

#include <3ds.h>

#define ROOT_PATH "/"
#define START_PATH ROOT_PATH
#define MAX_FILES 1024
#define FILES_PER_PAGE 5

#define wait(msec) svcSleepThread(10000000 * (s64)msec)

char cwd[512];
char fileName[255];

bool recycleBin, sysProtection, isHiddenEnabled;
int sortBy;

#endif