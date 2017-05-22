#include "fs.h"

void openSdArchive()
{
	FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
}

void closeSdArchive()
{
	FSUSER_CloseArchive(sdmcArchive);
}

const char *get_filename_ext(const char *filename) 
{
    const char *dot = strrchr(filename, '.');
    
	if (!dot || dot == filename) 
		return "";
    
	return dot + 1;
}

int makeDir(const char *path)
{
	if (!path) 
		return -1;
	
	return mkdir(path, 0777);
}

bool fileExists(char *path) 
{
    FILE * temp = fopen(path, "r");
    if(temp == NULL)
        return false;

    fclose(temp);

    return true;
}

bool dirExists(const char *path)
{
    struct stat info;

    if(stat( path, &info ) != 0)
        return false;
    else if(info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

char* getFileCreationTime(char *path) 
{
    struct stat attr;
    stat(path, &attr);
	
    return ctime(&attr.st_ctime);
}

char* getFileModifiedTime(char *path) 
{
    struct stat attr;
    stat(path, &attr);
	
    return ctime(&attr.st_mtime);
}

char* getFileAccessedTime(char *path) 
{
    struct stat attr;
    stat(path, &attr);
	
    return ctime(&attr.st_atime);
}

int getFileSize(const char *path)
{
	struct stat st;
	stat(path, &st);
	
	return st.st_size;
}