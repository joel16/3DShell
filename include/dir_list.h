#ifndef DIRLIST_H
#define DIRLIST_H

#include <3ds.h>

typedef struct File
{
	struct File * next; // Next item
	int isDir; // Folder flag
	int isReadOnly; // Read-only flag
	int isHidden; // Hidden file flag
	u8 name[256]; // File name
	char ext[4]; // File extension
	u64 size; // File size
} File;

extern File * files;

extern int position;
extern int fileCount;

Result Dirlist_PopulateFiles(bool clear);
void Dirlist_DisplayFiles(void);
File * Dirlist_GetFileIndex(int index);
void Dirlist_OpenFile(void);
int Dirlist_Navigate(bool folder);

#endif