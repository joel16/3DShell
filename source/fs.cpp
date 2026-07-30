#include <algorithm>
#include <cstring>
#include <filesystem>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "utils.h"

FS_Archive archive, sdmcArchive, nandArchive;

namespace FS {
    typedef struct {
        std::u16string path;
        std::u16string filename;
        bool isDir = false;
    } FSCopyEntry;
    
    static FSCopyEntry fsCopyEntry;
    static FS_Archive srcArchive;

    Result OpenArchive(FS_Archive *archive, FS_ArchiveID archiveID) {
        Result ret = 0;
        
        if (R_FAILED(ret = FSUSER_OpenArchive(archive, archiveID, fsMakePath(PATH_EMPTY, "")))) {
            return ret;
        }

        return 0;
    }

    Result CloseArchive(FS_Archive archive) {
        Result ret = 0;
        
        if (R_FAILED(ret = FSUSER_CloseArchive(archive))) {
            return ret;
        }

        return 0;
    }
    
    bool FileExists(FS_Archive archive, const std::u16string& path) {
        Handle handle;
        
        if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path.c_str()), FS_OPEN_READ, 0))) {
            return false;
        }
        
        if (R_FAILED(FSFILE_Close(handle))) {
            return false;
        }
        
        return true;
    }

    bool DirExists(FS_Archive archive, const std::u16string& path) {
        Handle handle;
        
        if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_UTF16, path.c_str())))) {
            return false;
        }
            
        if (R_FAILED(FSDIR_Close(handle))) {
            return false;
        }
            
        return true;
    }

    static void BuildPath(std::u16string& path, FS_DirectoryEntry *entry) {
        if (path != u"/" && !path.empty() && path.back() != u'/') {
            path += u'/';
        }

        if (entry != nullptr) {
            path.append(reinterpret_cast<const char16_t *>(entry->name));
        }
    }

    static void BuildPath(std::u16string& path, const std::u16string& filename) {
        if (path != u"/" && !path.empty() && path.back() != u'/') {
            path += u'/';
        }

        if (!filename.empty()) {
            path.append(filename);
        }
    }

    Result MakeDir(FS_Archive archive, const std::u16string& path) {
        Result ret = 0;
        
        if (R_FAILED(ret = FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path.c_str()), 0))) {
            return ret;
        }
        
        return 0;
    }
    
    static bool CompareEntry(const u16 *nameA, const u16 *nameB) {
        while (*nameA && *nameB) {
            char16_t lowerA = std::tolower(*nameA);
            char16_t lowerB = std::tolower(*nameB);
            
            if (lowerA < lowerB) {
                return true;
            }

            if (lowerA > lowerB) {
                return false;
            }
            
            ++nameA;
            ++nameB;
        }
        
        return *nameA < *nameB;
    }

    static u64 GetFreeStorage(FS_SystemMediaType mediatype) {
        Result ret = 0;
        FS_ArchiveResource resource = { 0 };
        
        if (R_FAILED(ret = FSUSER_GetArchiveResource(&resource, mediatype))) {
            Log::Error("FSUSER_GetArchiveResource(GetFreeStorage) failed: 0x%x\n", ret);
            return ret;
        }
            
        return (static_cast<u64>(resource.freeClusters) * static_cast<u64>(resource.clusterSize));
    }
    
    u64 GetTotalStorage(FS_SystemMediaType mediatype) {
        Result ret = 0;
        FS_ArchiveResource resource = { 0 };
        
        if (R_FAILED(ret = FSUSER_GetArchiveResource(&resource, mediatype))) {
            Log::Error("FSUSER_GetArchiveResource(GetTotalStorage) failed: 0x%x\n", ret);
            return ret;
        }
        
        return (static_cast<u64>(resource.totalClusters) * static_cast<u64>(resource.clusterSize));
    }
    
    u64 GetUsedStorage(FS_SystemMediaType mediatype) {
        Result ret = 0;
        FS_ArchiveResource resource = { 0 };
        
        if (R_FAILED(ret = FSUSER_GetArchiveResource(&resource, mediatype))) {
            Log::Error("FSUSER_GetArchiveResource(GetUsedStorage) failed: 0x%x\n", ret);
            return ret;
        }
            
        return ((static_cast<u64>(resource.totalClusters) * static_cast<u64>(resource.clusterSize)) - 
            (static_cast<u64>(resource.freeClusters) * static_cast<u64>(resource.clusterSize)));
    }
    
    static bool Sort(const FS_DirectoryEntry& entryA, const FS_DirectoryEntry& entryB) {
        bool isADir = entryA.attributes & FS_ATTRIBUTE_DIRECTORY;
        bool isBDir = entryB.attributes & FS_ATTRIBUTE_DIRECTORY;
        
        if (isADir && !isBDir) {
            return true;
        }
        
        if (!isADir && isBDir) {
            return false;
        }
        
        switch (cfg.sort) {
            case 0: // Sort alphabetically (ascending)
                return FS::CompareEntry(entryA.name, entryB.name);
                
            case 1: // Sort alphabetically (descending)
                return FS::CompareEntry(entryB.name, entryA.name);
                
            case 2: // Sort by file size (largest first)
                return entryB.fileSize < entryA.fileSize;
                
            case 3: // Sort by file size (smallest first)
                return entryA.fileSize < entryB.fileSize;
                
            default:
                return false;
        }
    }
    
    Result GetDirList(const std::u16string& path, std::vector<FS_DirectoryEntry>& entries, int filter) {
        entries.clear();
            
        Result ret = 0;
        Handle dir = 0;
        
        if (R_FAILED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_UTF16, path.c_str())))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
            Log::Error("FSUSER_OpenDirectory(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            return ret;
        }
        
        u32 count = 0;
        
        do {
            FS_DirectoryEntry entry;
            
            if (R_FAILED(ret = FSDIR_Read(dir, &count, 1, &entry))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
                Log::Error("FSDIR_Read(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
                FSDIR_Close(dir);
                return ret;
            }
            
            if (count == 1) {
                bool isDir = entry.attributes & FS_ATTRIBUTE_DIRECTORY;
                
                if (filter == -1 || isDir) {
                    entries.push_back(entry);
                }
                else {
                    FileType type = GetFileType(entry.shortExt);
                    if (static_cast<int>(type) == filter) {
                        entries.push_back(entry);
                    }
                }
            }
        } while(count > 0);
        
        std::sort(entries.begin(), entries.end(), FS::Sort);
        
        if (R_FAILED(ret = FSDIR_Close(dir))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
            Log::Error("FSDIR_Close(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            return ret;
        }
        
        return 0;
    }
    
    static Result ChangeDir(const std::u16string &path, std::vector<FS_DirectoryEntry> &entries) {
        Result ret = FS::GetDirList(path, entries);

        if (R_FAILED(ret)) {
            return ret;
        }
        
        cfg.cwd = path.empty() ? u"/" : path;
        return 0;
    }
    
    Result ChangeDirNext(const std::u16string &path, std::vector<FS_DirectoryEntry> &entries) {
        std::u16string newPath = cfg.cwd;
        FS::BuildPath(newPath, nullptr);
        
        if (!path.empty() && path.front() == u'/') {
            newPath += path.substr(1);
        }
        else {
            newPath += path;
        }
        
        return FS::ChangeDir(newPath, entries);
    }
    
    Result ChangeDirPrev(std::vector<FS_DirectoryEntry> &entries) {
        if (cfg.cwd == u"/" || cfg.cwd.empty()) {
            return -1;
        }
        
        size_t pos = cfg.cwd.find_last_of(u'/');
        
        if (pos == 0 || pos == std::u16string::npos) {
            cfg.cwd = u"/";
        }
        else {
            cfg.cwd.erase(pos);
        }
        
        return ChangeDir(cfg.cwd, entries);
    }
    
    const char* GetFileExt(const char *filename) {
        const char *ext = strrchr(filename, '.');
        
        if (ext == nullptr) {
            return "";
        }
        
        return ext + 1;
    }

    FileType GetFileType(const char *ext) {
        if (!ext || ext[0] == '\0') {
            return FileTypeNone;
        }

        // Archives
        if ((strncasecmp(ext, "zip", 3) == 0) || (strncasecmp(ext, "rar", 3) == 0) || (strncasecmp(ext, "7z", 2) == 0)
            || (strncasecmp(ext, "lzm", 3) == 0)) {
            return FileTypeArchive;
        }
        // Audio
        else if ((strncasecmp(ext, "fla", 3) == 0) || (strncasecmp(ext, "it", 2) == 0) || (strncasecmp(ext, "mod", 3) == 0)
            || (strncasecmp(ext, "mp3", 3) == 0) || (strncasecmp(ext, "ogg", 3) == 0) || (strncasecmp(ext, "opu", 3) == 0)
            || (strncasecmp(ext, "s3m", 3) == 0) || (strncasecmp(ext, "wav", 3) == 0) || (strncasecmp(ext, "xm", 2) == 0)) {
            return FileTypeAudio;
        }
        // Documents
        else if ((strncasecmp(ext, "cbt", 3) == 0) || (strncasecmp(ext, "cbz", 3) == 0) || (strncasecmp(ext, "epu", 3) == 0)
            || (strncasecmp(ext, "fb2", 3) == 0) || (strncasecmp(ext, "mob", 3) == 0) || (strncasecmp(ext, "pdf", 3) == 0)
            || (strncasecmp(ext, "xps", 3) == 0)) {
            return FileTypeDocument;
        }
        // Images
        if ((strncasecmp(ext, "bmp", 3) == 0) || (strncasecmp(ext, "gif", 3) == 0) || (strncasecmp(ext, "jpg", 3) == 0)
            || (strncasecmp(ext, "jpe", 3) == 0) || (strncasecmp(ext, "pgm", 3) == 0) || (strncasecmp(ext, "ppm", 3) == 0)
            || (strncasecmp(ext, "png", 3) == 0) || (strncasecmp(ext, "psd", 3) == 0) || (strncasecmp(ext, "tga", 3) == 0)
            || (strncasecmp(ext, "web", 3) == 0)) {
            return FileTypeImage;
        }
        // Text
        else if ((strncasecmp(ext, "jso", 3) == 0) || (strncasecmp(ext, "log", 3) == 0) || (strncasecmp(ext, "txt", 3) == 0)
            || (strncasecmp(ext, "cfg", 3) == 0) || (strncasecmp(ext, "ini", 3) == 0)) {
            return FileTypeText;
        }
            
        return FileTypeNone;
    }
    
    void GetUTF8Path(char *out, size_t size, const u16 *filename) {
        std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(cfg.cwd.c_str()));
        std::string utf8filename = Utils::UTF16ToUTF8(filename);
        
        if (!utf8Path.empty() && utf8Path.back() != '/') {
            utf8Path += '/';
        }
        
        std::snprintf(out, size, "%s%s", utf8Path.c_str(), utf8filename.c_str());
    }
    
    const char *GetFilename(const char *path) {
        const char *lastSlash = strrchr(path, '/');

        if (lastSlash == nullptr) {
            return path;
        }
        
        return lastSlash + 1;
    }

    Result Rename(FS_DirectoryEntry *entry, const std::u16string& filename) {
        Result ret = 0;
        std::u16string path = cfg.cwd;
        FS::BuildPath(path, entry);
        
        std::u16string newPath = cfg.cwd;
        FS::BuildPath(newPath, filename);
        
        if (entry->attributes & FS_ATTRIBUTE_DIRECTORY) {
            if (R_FAILED(ret = FSUSER_RenameDirectory(archive, fsMakePath(PATH_UTF16, path.c_str()), archive, fsMakePath(PATH_UTF16, newPath.c_str())))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
                std::string utf8NewPath = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(newPath.c_str()));
                Log::Error("FSUSER_RenameDirectory(%s, %s) failed: 0x%x\n", utf8Path.c_str(), utf8NewPath.c_str(), ret);
                return ret;
            }
        }
        else {
            if (R_FAILED(ret = FSUSER_RenameFile(archive, fsMakePath(PATH_UTF16, path.c_str()), archive, fsMakePath(PATH_UTF16, newPath.c_str())))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
                std::string utf8NewPath = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(newPath.c_str()));
                Log::Error("FSUSER_RenameFile(%s, %s) failed: 0x%x\n", utf8Path.c_str(), utf8NewPath.c_str(), ret);
                return ret;
            }
        }
        
        return 0;
    }
    
    static void ClearFSCopyEntry(void) {
        fsCopyEntry.path.clear();
        fsCopyEntry.filename.clear();
        fsCopyEntry.isDir = false;
    }

    static Result CopyFile(const std::u16string &srcPath, const std::u16string &destPath) {
        Result ret = 0;
        Handle srcHandle, destHandle;
        
        if (R_FAILED(ret = FSUSER_OpenFile(&srcHandle, srcArchive, fsMakePath(PATH_UTF16, srcPath.c_str()), FS_OPEN_READ, 0))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()));
            Log::Error("FSUSER_OpenFile(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            return ret;
        }
        
        u64 size = 0;
        if (R_FAILED(ret = FSFILE_GetSize(srcHandle, &size))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()));
            Log::Error("FSFILE_GetSize(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            FSFILE_Close(srcHandle);
            return ret;
        }

        // Make sure we have enough storage to carry out this operation
        if (FS::GetFreeStorage(srcArchive == sdmcArchive? SYSTEM_MEDIATYPE_SD : SYSTEM_MEDIATYPE_CTR_NAND) < size) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()));
            Log::Error("Not enough storage is available to process this command 0x%x\n", utf8Path.c_str(), ret);
            FSFILE_Close(srcHandle);
            return -1;
        }
        
        // This may fail or not, but we don't care -> create the file if it doesn't exist, otherwise continue.
        FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, destPath.c_str()), 0, size);
        
        if (R_FAILED(ret = FSUSER_OpenFile(&destHandle, archive, fsMakePath(PATH_UTF16, destPath.c_str()), FS_OPEN_WRITE, 0))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(destPath.c_str()));
            Log::Error("FSUSER_OpenFile(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            FSFILE_Close(srcHandle);
            return ret;
        }
        
        u32 bytesRead = 0, bytesWritten = 0;
        const u64 bufferSize = 0x10000;
        u64 offset = 0;
        u8 *buf = new u8[bufferSize];
        std::string filename = std::filesystem::path(Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()))).filename();
        
        do {
            if (Utils::IsCancelButtonPressed()) {
                FSFILE_SetSize(destHandle, offset);
                delete[] buf;
                FSFILE_Close(srcHandle);
                FSFILE_Close(destHandle);
                return 0;
            }
            
            std::memset(buf, 0, bufferSize);
            
            if (R_FAILED(ret = FSFILE_Read(srcHandle, &bytesRead, offset, buf, bufferSize))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()));
                Log::Error("FSFILE_Read(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
                delete[] buf;
                FSFILE_Close(srcHandle);
                FSFILE_Close(destHandle);
                return ret;
            }
            
            if (R_FAILED(ret = FSFILE_Write(destHandle, &bytesWritten, offset, buf, bytesRead, FS_WRITE_FLUSH))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(destPath.c_str()));
                Log::Error("FSFILE_Write(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
                delete[] buf;
                FSFILE_Close(srcHandle);
                FSFILE_Close(destHandle);
                return ret;
            }
            
            offset += bytesRead;
            GUI::ProgressBar("Copying", filename.c_str(), offset, size);
        } while(offset < size);
        
        delete[] buf;
        FSFILE_Close(srcHandle);
        FSFILE_Close(destHandle);
        return 0;
    }

    static Result CopyDir(const std::u16string &srcPath, const std::u16string &destPath) {
        Result ret = 0;
        Handle dir;
        
        if (R_FAILED(ret = FSUSER_OpenDirectory(&dir, srcArchive, fsMakePath(PATH_UTF16, srcPath.c_str())))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()));
            Log::Error("FSUSER_OpenDirectory(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            return ret;
        }
        
        // This may fail or not, but we don't care -> make the dir if it doesn't exist, otherwise continue.
        FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, destPath.c_str()), 0);
        
        u32 count = 0;
        
        do {
            FS_DirectoryEntry entry;
            
            if (R_FAILED(ret = FSDIR_Read(dir, &count, 1, &entry))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()));
                Log::Error("FSDIR_Read(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
                return ret;
            }
            
            if (count == 1) {
                std::u16string src = srcPath;
                src.append(u"/");
                src.append(reinterpret_cast<const char16_t *>(entry.name));
                
                std::u16string dest = destPath;
                dest.append(u"/");
                dest.append(reinterpret_cast<const char16_t *>(entry.name));
                
                if (entry.attributes & FS_ATTRIBUTE_DIRECTORY) {
                    FS::CopyDir(src, dest);
                }
                else {
                    FS::CopyFile(src, dest);
                }
            }
        } while(count > 0);
        
        if (R_FAILED(ret = FSDIR_Close(dir))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(srcPath.c_str()));
            Log::Error("FSDIR_Close(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            return ret;
        }
        
        return 0;
    }
    
    void Copy(FS_DirectoryEntry *entry, const std::u16string& path) {
        FS::ClearFSCopyEntry();
        fsCopyEntry.path = path;
        FS::BuildPath(fsCopyEntry.path, entry);
        fsCopyEntry.filename = reinterpret_cast<const char16_t *>(entry->name);
        
        if (entry->attributes & FS_ATTRIBUTE_DIRECTORY) {
            fsCopyEntry.isDir = true;
        }
            
        srcArchive = archive;
    }
    
    Result Paste(void) {
        Result ret = 0;
        std::u16string path = cfg.cwd;
        FS::BuildPath(path, fsCopyEntry.filename);
        
        if (fsCopyEntry.isDir) {
            ret = FS::CopyDir(fsCopyEntry.path, path);
        }
        else {
            ret = FS::CopyFile(fsCopyEntry.path, path);
        }
            
        FS::ClearFSCopyEntry();
        return ret;
    }
    
    Result Move(void) {
        Result ret = 0;
        std::u16string path = cfg.cwd;
        FS::BuildPath(path, fsCopyEntry.filename);
        
        if (fsCopyEntry.isDir) {
            if (R_FAILED(ret = FSUSER_RenameDirectory(srcArchive, fsMakePath(PATH_UTF16, fsCopyEntry.path.c_str()), archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
                std::string utf8Filename = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(fsCopyEntry.filename.c_str()));
                Log::Error("FSUSER_RenameDirectory(%s, %s) failed: 0x%x\n", utf8Path.c_str(), utf8Filename.c_str(), ret);
                return ret;
            }
        }
        else {
            if (R_FAILED(ret = FSUSER_RenameFile(srcArchive, fsMakePath(PATH_UTF16, fsCopyEntry.path.c_str()), archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
                std::string utf8Filename = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(fsCopyEntry.filename.c_str()));
                Log::Error("FSUSER_RenameFile(%s, %s) failed: 0x%x\n", utf8Path.c_str(), utf8Filename.c_str(), ret);
                return ret;
            }
        }
        
        FS::ClearFSCopyEntry();
        return 0;
    }

    Result Delete(FS_DirectoryEntry& entry) {
        Result ret = 0;
        std::u16string path = cfg.cwd;
        FS::BuildPath(path, std::addressof(entry));
        
        if (entry.attributes & FS_ATTRIBUTE_DIRECTORY) {
            if (R_FAILED(ret = FSUSER_DeleteDirectoryRecursively(archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
                Log::Error("FSUSER_DeleteDirectoryRecursively(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
                return ret;
            }
        }
        else {
            if (R_FAILED(ret = FSUSER_DeleteFile(archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
                Log::Error("FSUSER_DeleteFile(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
                return ret;
            }
        }
        
        return 0;
    }

    Result CreateFile(const std::u16string& filename) {
        Result ret = 0;
        std::u16string path = cfg.cwd;
        FS::BuildPath(path, filename);

        if (R_FAILED(ret = FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path.c_str()), 0, 0))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
            Log::Error("FSUSER_CreateFile(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            return ret;
        }

        return 0;
    }

    Result CreateFolder(const std::u16string& filename) {
        Result ret = 0;
        std::u16string path = cfg.cwd;
        FS::BuildPath(path, filename);

        if (R_FAILED(ret = FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path.c_str()), 0))) {
            std::string utf8Path = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
            Log::Error("FSUSER_CreateDirectory(%s) failed: 0x%x\n", utf8Path.c_str(), ret);
            return ret;
        }

        return 0;
    }
}
