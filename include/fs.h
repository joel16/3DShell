#ifndef FS_H
#define FS_H

#include <3ds.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

FS_Archive sdmcArchive;

void openSdArchive();
void closeSdArchive();
const char *get_filename_ext(const char *filename);
int makeDir(const char * path);
bool fileExists(char * path);
bool dirExists(const char * path);
char* getFileCreationTime(char *path);
char* getFileModifiedTime(char *path);
char* getFileAccessedTime(char *path);
int getFileSize(const char *path);

#endif