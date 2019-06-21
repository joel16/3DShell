#ifndef _3DSHELL_COMMON_H
#define _3DSHELL_COMMON_H

#include <setjmp.h>

#define wait(msec) svcSleepThread(10000000 * (s64)msec)

#define ROOT_PATH "/"
#define START_PATH ROOT_PATH
#define MAX_FILES 1024

enum MENU_STATES {
    MENU_STATE_HOME = 0,
    MENU_STATE_FILEOPTIONS = 1,
    MENU_STATE_MENUBAR = 2,
    MENU_STATE_SETTINGS = 3,
    MENU_STATE_FTP = 4,
    MENU_STATE_SORT = 5,
    MENU_STATE_DELETE = 6,
    MENU_STATE_PROPERTIES = 7,
    MENU_STATE_UPDATE = 8,
    MENU_STATE_UPDATE_2 = 9,
    MENU_STATE_ABOUT = 10
};

enum BROWSE_STATES {
    BROWSE_STATE_SD = 0,
    BROWSE_STATE_NAND = 1
};

extern jmp_buf exitJmp;

extern int MENU_STATE;
extern int BROWSE_STATE;

extern char cwd[512];

#endif
