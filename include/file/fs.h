#ifndef FS_H
#define FS_H

#include <3ds.h>

#include <string.h>
#include <time.h>

FS_Archive fsArchive;

int BROWSE_STATE;

#define STATE_SD 0
#define STATE_NAND 1

typedef struct 
{
    Handle handle;
    u64 offset;
	u64 size;
	unsigned int error;
} FSFILE;

Result openArchive(FS_Archive * archive, FS_ArchiveID id);
Result closeArchive(FS_Archive archive);
Result makeDir(FS_Archive archive, const char * path);
bool fileExists(FS_Archive archive, const char * path);
bool dirExists(FS_Archive archive, const char * path);
char* getFileModifiedTime(char * path);
u64 getFileSize(FS_Archive archive, const char * path);
Result fsRemove(FS_Archive archive, const char * filename);
Result fsRmdir(FS_Archive archive, const char * path);
Result fsRmdirRecursive(FS_Archive archive, const char * path);
Result fsRenameFile(FS_Archive archive, const char * old_filename, const char * new_filename);
Result fsRenameDir(FS_Archive archive, const char * old_filename, const char * new_filename);
Result fsOpen(Handle * handle, const char * path, u32 flags);
Result writeFile(const char * path, const char * buf);

#endif