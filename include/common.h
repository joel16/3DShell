#ifndef NX_SHELL_COMMON_H
#define NX_SHELL_COMMON_H

#include <setjmp.h>

#define wait(msec) svcSleepThread(10000000 * (s64)msec)

#define ROOT_PATH "/"
#define START_PATH ROOT_PATH
#define MAX_FILES 1024
#define FILES_PER_PAGE 5

#define MENU_STATE_HOME       0
#define MENU_STATE_OPTIONS    1
#define MENU_STATE_MENUBAR    2
#define MENU_STATE_SETTINGS   3
#define MENU_STATE_FTP        4
#define MENU_STATE_SORT       5
#define MENU_STATE_THEMES     6
#define MENU_STATE_DIALOG     7
#define MENU_STATE_PROPERTIES 8

#define STATE_SD   0
#define STATE_NAND 1

jmp_buf exitJmp;

int MENU_DEFAULT_STATE;
int BROWSE_STATE;

char cwd[512];
char fileName[256];

#endif