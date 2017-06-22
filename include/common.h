#ifndef COMMON_H
#define COMMON_H

#include <3ds.h>
#include <setjmp.h>
#include <sf2d.h>
#include <sfil.h>
#include <sftd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROOT_PATH "/"
#define START_PATH ROOT_PATH

/*
*	Current working directory.
*/
char cwd[1024];

/*
*	Current file name.
*/
char fileName[1024];

/*
*	Generic definitions
*/
bool copyF, cutF, properties, deleteDialog, bgmEnable, sysProtection;

jmp_buf exitJmp; 

sf2d_texture	*background, *options, *_properties, *selector, *deletion, *folderIcon, *fileIcon, *uncheck,
				*audioIcon, *appIcon, *txtIcon, *systemIcon, *zipIcon, *imgIcon, *homeIcon, *optionsIcon, *sdIcon, *nandIcon,
				*ftpIcon, *s_HomeIcon, *s_OptionsIcon, *s_SdIcon, *s_NandIcon, *settingsIcon, *s_SettingsIcon, *s_ftpIcon,
				*searchIcon, *updateIcon, *s_UpdateIcon, *toggleOn, *toggleOff, *themeIcon, *dlIcon, *s_dlIcon;

sftd_font	*font, *font2;

touchPosition touch;

#define kPressed hidKeysDown()
#define kHeld hidKeysHeld()

#endif