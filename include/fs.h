#pragma once

#include <3ds.h>
#include <string>
#include <vector>

extern FS_Archive archive, sdmcArchive, nandArchive;

typedef enum FileType {
    FileTypeNone,
    FileTypeArchive,
    FileTypeAudio,
    FileTypeDocument,
    FileTypeImage,
    FileTypeText,
} FileType;

namespace FS {
    Result OpenArchive(FS_Archive *archive, FS_ArchiveID archiveID);
    Result CloseArchive(FS_Archive archive);
    bool FileExists(FS_Archive archive, const std::u16string& path);
    bool DirExists(FS_Archive archive, const std::u16string& path);
    Result MakeDir(FS_Archive archive, const std::u16string& path);
    u64 GetTotalStorage(FS_SystemMediaType mediatype);
    u64 GetUsedStorage(FS_SystemMediaType mediatype);
    Result GetDirList(const std::u16string& path, std::vector<FS_DirectoryEntry>& entries, int filter = -1);
    Result ChangeDirNext(const std::u16string &path, std::vector<FS_DirectoryEntry> &entries);
    Result ChangeDirPrev(std::vector<FS_DirectoryEntry> &entries);
    const char* GetFileExt(const char *filename);
    FileType GetFileType(const char *filename);
    void GetUTF8Path(char *out, size_t size, const u16 *filename);
    const char *GetFilename(const char *path);
    Result Rename(FS_DirectoryEntry *entry, const std::u16string& filename);
    void Copy(FS_DirectoryEntry *entry, const std::u16string& path);
    Result Paste(void);
    Result Move(void);
    Result Delete(FS_DirectoryEntry& entry);
    Result CreateFile(const std::u16string& filename);
    Result CreateFolder(const std::u16string& filename);
}
