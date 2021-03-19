#ifndef _3D_SHELL_FS_H
#define _3D_SHELL_FS_H

#include <3ds.h>
#include <string>
#include <vector>

extern FS_Archive archive, sdmc_archive, nand_archive;

typedef enum FileType {
    FileTypeNone,
    FileTypeArchive,
    FileTypeImage,
    FileTypeText,
    FileTypeZip
} FileType;

namespace FS {
    Result OpenArchive(FS_Archive *archive, FS_ArchiveID id);
    Result CloseArchive(FS_Archive archive);
    bool FileExists(FS_Archive archive, const std::string &path);
    bool DirExists(FS_Archive archive, const std::string &path);
    std::string GetFileExt(const std::string &filename);
    u64 GetTotalStorage(FS_SystemMediaType mediatype);
    u64 GetUsedStorage(FS_SystemMediaType mediatype);
    FileType GetFileType(const std::string &filename);
    Result GetDirList(const std::string &path, std::vector<FS_DirectoryEntry> &entries);
    Result ChangeDirNext(const std::string &path, std::vector<FS_DirectoryEntry> &entries);
    Result ChangeDirPrev(std::vector<FS_DirectoryEntry> &entries);
    Result Delete(FS_DirectoryEntry *entry);
    Result Rename(FS_DirectoryEntry *entry, const std::string &filename);
    void Copy(FS_DirectoryEntry *entry, const std::string &path);
    Result Paste(void);
    Result Move(void);
}

#endif
