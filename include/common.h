#ifndef COMMON_H
#define COMMON_H

#include <3ds.h>

#include <setjmp.h>

#define ROOT_PATH "/"
#define START_PATH ROOT_PATH

#define TEXTURE_BACKGROUND 0
#define TEXTURE_SELECTOR 1
#define TEXTURE_OPTIONS 2
#define TEXTURE_PROPERTIES 3
#define TEXTURE_DELETE 4

#define TEXTURE_FOLDER_ICON 5
#define TEXTURE_FILE_ICON 6
#define TEXTURE_APP_ICON 7
#define TEXTURE_AUDIO_ICON 8
#define TEXTURE_IMG_ICON 9
#define TEXTURE_SYSTEM_ICON 10
#define TEXTURE_TXT_ICON 11
#define TEXTURE_ZIP_ICON 12

#define TEXTURE_HOME_ICON 13
#define TEXTURE_OPTIONS_ICON 14
#define TEXTURE_SETTINGS_ICON 15
#define TEXTURE_UPDATE_ICON 16
#define TEXTURE_FTP_ICON 17

#define TEXTURE_HOME_ICON_SELECTED 18
#define TEXTURE_OPTIONS_ICON_SELECTED  19
#define TEXTURE_SETTINGS_ICON_SELECTED  20
#define TEXTURE_UPDATE_ICON_SELECTED  21
#define TEXTURE_FTP_ICON_SELECTED  22

#define TEXTURE_SD_ICON 23
#define TEXTURE_NAND_ICON 24

#define TEXTURE_SD_ICON_SELECTED 25
#define TEXTURE_NAND_ICON_SELECTED 26

#define TEXTURE_TOGGLE_ON 27
#define TEXTURE_TOGGLE_OFF 28

#define TEXTURE_CHECK_ICON 29
#define TEXTURE_UNCHECK_ICON 30

#define TEXTURE_SEARCH_ICON 31

#define TEXTURE_THEME_ICON 32

/*
*	Current working directory.
*/
char cwd[255];

/*
*	Current file name.
*/
char fileName[255];

/*
*	Generic definitions
*/
bool copyF, cutF, properties, deleteDialog, recycleBin, sysProtection, isHiddenEnabled;

jmp_buf exitJmp;

touchPosition touch;

#define kPressed (u32)hidKeysDown()
#define kHeld (u32)hidKeysHeld()

#endif