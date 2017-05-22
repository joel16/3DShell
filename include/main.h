#ifndef MAIN_H
#define MAIN_H

#include <3ds.h>
#include <dirent.h>
#include <sf2d.h>
#include <sfil.h>
#include <sftd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>

#include "Roboto_ttf.h"

#define ROOT_PATH "/"
#define START_PATH ROOT_PATH

sf2d_texture	*background, *options, *_properties, *selector, *deletion, *folderIcon, *fileIcon, *uncheck,
				*audioIcon, *appIcon, *txtIcon, *systemIcon, *zipIcon, *imgIcon, *homeIcon, *optionsIcon, *sdIcon, *nandIcon,
				*selectedHomeIcon, *selectedOptionsIcon, *selectedSdIcon, *selectedNandIcon, *settingsIcon, *selectedSettingsIcon, *searchIcon;

sftd_font	*font, *font2;

/*
*	Current working directory.
*/
char cwd[1024];

/*
*	Current file name.
*/
char fileName[1024];

/*
*	Copy Move Origin
*/
char copysource[1024];

/*
*	Username
*/
char userName[20];

/*
*	A greeting.
*/
char welcomeMsg[100];

/*
*	Current Date
*/
char currDate[100];

/*
*	Copy Flags
*/
#define COPY_FOLDER_RECURSIVE 2
#define COPY_DELETE_ON_FINISH 1
#define COPY_KEEP_ON_FINISH 0
#define NOTHING_TO_COPY -1

/*
*	Generic definitions
*/
bool copyF, cutF, properties, deleteDialog;
int selectionX, selectionY, BROWSE_STATE, DEFAULT_STATE;

#define STATE_HOME 0
#define STATE_OPTIONS 1
#define STATE_SETTINGS 2
#define STATE_SD 0
#define STATE_NAND 1
#define IF_OPTIONS ((DEFAULT_STATE != STATE_HOME) && (DEFAULT_STATE != STATE_SETTINGS))

#define wait(nanoSec) svcSleepThread(nanoSec);

#define resetSelection() selectionX = -1, selectionY = -1;

#define CLEAR 1
#define KEEP 0

#define FILES_PER_PAGE 5

/*
*	File struct
*/
typedef struct File
{
	// Next Item
	struct File * next;

	// Folder Flag
	int isFolder;

	// File Name
	char name[256];
} File;

void mainMenu(int clearindex);
void updateList(int clearindex);
void displayFiles(int withclear);
void recursiveFree(File * node);
void open(void);
int navigate(int _case);
File * findindex(int index);
int drawDeletionDialog();
int displayProperties();
void newFolder();
int renameF();
int delete(void);
void copy(int flag);
int copy_file(char * a, char * b);
int copy_folder_recursive(char * a, char * b);
int paste(void);

#endif