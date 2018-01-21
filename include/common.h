#ifndef COMMON_H
#define COMMON_H

#include <3ds.h>

#define ROOT_PATH "/"
#define START_PATH ROOT_PATH
#define MAX_FILES 1024
#define FILES_PER_PAGE 5

#define wait(msec) svcSleepThread(10000000 * (s64)msec)

int MENU_DEFAULT_STATE;

#define MENU_STATE_HOME     0
#define MENU_STATE_OPTIONS  1
#define MENU_STATE_SETTINGS 2
#define MENU_STATE_FTP      3
#define MENU_STATE_SORT     4
#define MENU_STATE_THEMES   5

int BROWSE_STATE;

#define STATE_SD   0
#define STATE_NAND 1

char cwd[512];
char fileName[255];

bool recycleBin, sysProtection, isHiddenEnabled;
int sortBy;

#endif