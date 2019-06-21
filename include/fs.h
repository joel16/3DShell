#ifndef _3D_SHELL_FS_H
#define _3D_SHELL_FS_H

#include <3ds.h>

extern FS_Archive archive, sdmc_archive, nand_archive;

Result FS_OpenArchive(FS_Archive *archive, FS_ArchiveID id);
Result FS_CloseArchive(FS_Archive archive);
Result FS_OpenDir(Handle *handle, FS_Archive archive, const char *path);
Result FS_OpenFile(Handle *handle, FS_Archive archive, const char *path, u32 flags, u32 attributes);
Result FS_MakeDir(FS_Archive archive, const char *path);
Result FS_CreateFile(FS_Archive archive, const char *path);
Result FS_RecursiveMakeDir(FS_Archive archive, const char *path);
bool FS_FileExists(FS_Archive archive, const char *path);
bool FS_DirExists(FS_Archive archive, const char *path);
Result FS_GetFileSize(FS_Archive archive, const char *path, u64 *size);
u64 FS_GetFreeStorage(FS_SystemMediaType media_type);
u64 FS_GetTotalStorage(FS_SystemMediaType media_type);
u64 FS_GetUsedStorage(FS_SystemMediaType media_type);
Result FS_RemoveFile(FS_Archive archive, const char *path);
Result FS_RemoveDir(FS_Archive archive, const char *path);
Result FS_RemoveDirRecursive(FS_Archive archive, const char *path);
Result FS_RenameFile(FS_Archive archive, const char *old_filename, const char *new_filename);
Result FS_RenameDir(FS_Archive archive, const char *old_dirname, const char *new_dirname);
Result FS_Read(FS_Archive archive, const char *path, u64 size, void *buf);
Result FS_Write(FS_Archive archive, const char *path, const void *buf);
char *FS_GetFileTimestamp(const char *path);

#endif
