#ifndef _3D_SHELL_DIRBROWSE_H
#define _3D_SHELL_DIRBROWSE_H

#include <3ds.h>

typedef struct File {
	struct File *next; // Next item
	int isDir;          // Folder flag
	int isReadOnly;     // Read-only flag
	int isHidden;       // Hidden file flag
	char name[256];       // File name
	char ext[4];        // File extension
	u64 size;           // File size
} File;

extern File *files;
extern int position;
extern int file_count;

extern int multi_select_index;           // Multi-select index.
extern bool multi_select[256];           // Array of indices selected.
extern int multi_select_indices[256];    // Array to hold the indices.
extern char multi_select_dir[256];       // Holds the current dir where multi-select happens.
extern char multi_select_paths[51][256]; // Holds the file paths of those in the clipboard.

void Dirbrowse_RecursiveFree(File *node);
Result Dirbrowse_PopulateFiles(bool clear);
void Dirbrowse_DisplayFiles(void);
File *Dirbrowse_GetFileIndex(int index);
void Dirbrowse_OpenFile(void);
int Dirbrowse_Navigate(bool parent);

#endif
