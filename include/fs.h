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
} FSFILE;

void openSdArchive();
void closeSdArchive();
const char *get_filename_ext(const char *filename);
int makeDir(const char * path);
bool fileExists(char * path);
bool dirExists(const char * path);
char* getFileCreationTime(char *path);
char* getFileModifiedTime(char *path);
char* getFileAccessedTime(char *path);
u64 getFileSize(FS_Archive archive, const char *path);
int fsRemove(FS_Archive archive, const char *filename);
int fsRename(FS_Archive archive, const char *old_filename, const char *new_filename);

#endif