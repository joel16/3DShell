#ifndef DIRLIST_H
#define DIRLIST_H

#include <fcntl.h>
#include <unistd.h>

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
	
	// Read-only Flag
	int isRDONLY;

	// File Name
	char name[256];
	
	// File Extension
	char ext[4];
} File;

/*
*	Menu position
*/
extern int position;

/*
*	Number of files
*/
extern int fileCount;

/*
*	File list
*/
extern File * files;

/*
*	Generic definitions
*/
int selectionX, selectionY, BROWSE_STATE, DEFAULT_STATE;

#define STATE_HOME 0
#define STATE_OPTIONS 1
#define STATE_SETTINGS 2
#define STATE_UPDATE 3
#define STATE_FTP 4
#define STATE_THEME 5
#define STATE_DOWNLOAD 6

#define STATE_SD 0
#define STATE_NAND 1

#define IF_OPTIONS	((DEFAULT_STATE != STATE_HOME) && (DEFAULT_STATE != STATE_UPDATE) && (DEFAULT_STATE != STATE_SETTINGS) && \
					(DEFAULT_STATE != STATE_FTP) && (DEFAULT_STATE != STATE_THEME))
					
#define IF_SETTINGS ((DEFAULT_STATE != STATE_HOME) && (DEFAULT_STATE != STATE_UPDATE) && (DEFAULT_STATE != STATE_OPTIONS) && \
					(DEFAULT_STATE != STATE_FTP) && (DEFAULT_STATE != STATE_THEME))

#define IF_THEME 	(DEFAULT_STATE == STATE_THEME)
					
#define CAN_COPY 	(((copyF == false) && (cutF != true) && (deleteDialog == false)))
#define CAN_CUT 	(((cutF == false) && (copyF != true) && (deleteDialog == false)))

#define SYS_FILES 	((strcmp(file->name, "boot.firm") == 0) || (strcmp(file->name, "boot9strap") == 0) || \
					(strcmp(getLastNChars(file->name, 12), "Nintendo 3DS") == 0) || (strcmp(file->name, "arm9loaderhax.bin") == 0) || \
					(strncmp(cwd, "/Nintendo 3DS/", 14) == 0) || (strncmp(cwd, "/boot9strap/", 12) == 0))
					
#define resetSelection() selectionX = -1, selectionY = -1;

void updateList(int clearindex);
void displayFiles();
void recursiveFree(File * node);
void openFile(void);
int navigate(int _case);
File * findindex(int index);
int drawDeletionDialog();
int displayProperties();

#endif