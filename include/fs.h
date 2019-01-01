#ifndef _3D_SHELL_FS_H
#define _3D_SHELL_FS_H

#include <3ds.h>

FS_Archive archive, sdmc_archive, nand_archive;

Result FS_OpenArchive(FS_Archive *archive, FS_ArchiveID id);
Result FS_CloseArchive(FS_Archive archive);
Result FS_MakeDir(FS_Archive archive, const char *path);
Result FS_CreateFile(FS_Archive archive, const char *path);
Result FS_RecursiveMakeDir(FS_Archive archive, const char *dir);
bool FS_FileExists(FS_Archive archive, const char *path);
bool FS_DirExists(FS_Archive archive, const char *path);
char* FS_GetFileModifiedTime(char *path);
Result FS_GetFileSize(FS_Archive archive, const char *path, u64 *size);
u64 FS_GetFreeStorage(FS_SystemMediaType mediaType);
u64 FS_GetTotalStorage(FS_SystemMediaType mediaType);
u64 FS_GetUsedStorage(FS_SystemMediaType mediaType);
Result FS_Remove(FS_Archive archive, const char *path);
Result FS_Rmdir(FS_Archive archive, const char *path);
Result FS_RmdirRecursive(FS_Archive archive, const char *path);
Result FS_RenameFile(FS_Archive archive, const char *old_filename, const char *new_filename);
Result FS_RenameDir(FS_Archive archive, const char *old_dirname, const char *new_dirname);
Result FS_Open(Handle *handle, FS_Archive archive, const char *path, u32 flags);
Result FS_Read(FS_Archive archive, const char *path, u64 size, char *buf);
Result FS_Write(FS_Archive archive, const char *path, const char *buf);
char *FS_GetFileTimestamp(char *path);

#endif
