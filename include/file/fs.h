#ifndef FS_H
#define FS_H

#include <3ds.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

FS_Archive sdmcArchive;

typedef struct 
{
    Handle handle;
    u64 offset;
	u64 size;
	unsigned int error;
} FSFILE;

void openSdArchive();
void closeSdArchive();
int makeDir(FS_Archive archive, const char *path);
bool fileExists(FS_Archive archive, char * path);
bool dirExists(FS_Archive archive, const char * path);
char* getFileCreationTime(char *path);
char* getFileModifiedTime(char *path);
char* getFileAccessedTime(char *path);
u64 getFileSize(FS_Archive archive, const char *path);
int fsRemove(FS_Archive archive, const char *filename);
int fsRmdir(FS_Archive archive, const char * path);
int fsRename(FS_Archive archive, const char *old_filename, const char *new_filename);

#endif