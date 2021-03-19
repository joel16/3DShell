#include <algorithm>
#include <codecvt>
#include <cstring>
#include <filesystem>
#include <locale>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"

FS_Archive archive, sdmc_archive, nand_archive;

namespace FS {
    static FS_Archive src_archive;
    
    typedef struct {
        std::u16string  copy_path;
        std::u16string copy_filename;
        bool is_dir = false;
    } FSCopyEntry;
    
    FSCopyEntry fs_copy_entry;

    Result OpenArchive(FS_Archive *archive, FS_ArchiveID id) {
        Result ret = 0;
        
        if (R_FAILED(ret = FSUSER_OpenArchive(archive, id, fsMakePath(PATH_EMPTY, ""))))
            return ret;
            
        return 0;
    }
    
    Result CloseArchive(FS_Archive archive) {
        Result ret = 0;
        
        if (R_FAILED(ret = FSUSER_CloseArchive(archive)))
            return ret;
            
        return 0;
    }
    
    bool FileExists(FS_Archive archive, const std::string &path) {
        Handle handle;
        std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());
        
        if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path_u16.c_str()), FS_OPEN_READ, 0)))
            return false;
            
        if (R_FAILED(FSFILE_Close(handle)))
            return false;
            
        return true;
    }
    
    bool DirExists(FS_Archive archive, const std::string &path) {
        Handle handle;
        std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());
        
        if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_UTF16, path_u16.c_str()))))
            return false;
            
        if (R_FAILED(FSDIR_Close(handle)))
            return false;
            
        return true;
    }
    
    std::string GetFileExt(const std::string &filename) {
        std::string ext = std::filesystem::path(filename).extension();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
        return ext;
    }
    
    FileType GetFileType(const std::string &filename) {
        std::string ext = FS::GetFileExt(filename);

        if ((!ext.compare(".BMP")) || (!ext.compare(".GIF")) || (!ext.compare(".JPG")) || (!ext.compare(".JPEG")) || (!ext.compare(".PGM"))
            || (!ext.compare(".PPM")) || (!ext.compare(".PNG")) || (!ext.compare(".PSD")) || (!ext.compare(".TGA")) || (!ext.compare(".WEBP")))
            return FileTypeImage;
        else if ((!ext.compare(".JSON")) || (!ext.compare(".LOG")) || (!ext.compare(".TXT")) || (!ext.compare(".CFG")) || (!ext.compare(".INI")))
            return FileTypeText;
        else if ((!ext.compare(".ZIP")) || (!ext.compare(".RAR")) || (!ext.compare(".7Z")) || (!ext.compare(".LZMA")))
            return FileTypeZip;
            
        return FileTypeNone;
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
    
    static bool Sort(const FS_DirectoryEntry &entryA, const FS_DirectoryEntry &entryB) {
        if ((entryA.attributes & FS_ATTRIBUTE_DIRECTORY) && !(entryB.attributes & FS_ATTRIBUTE_DIRECTORY))
            return true;
        else if (!(entryA.attributes & FS_ATTRIBUTE_DIRECTORY) && (entryB.attributes & FS_ATTRIBUTE_DIRECTORY))
            return false;
        else {
            std::u16string entryA_name = reinterpret_cast<const char16_t *>(entryA.name);
            std::u16string entryB_name = reinterpret_cast<const char16_t *>(entryB.name);
            std::transform(entryA_name.begin(), entryA_name.end(), entryA_name.begin(), [](unsigned char c){ return std::tolower(c); });
            std::transform(entryB_name.begin(), entryB_name.end(), entryB_name.begin(), [](unsigned char c){ return std::tolower(c); });

            switch(cfg.sort) {
                case 0: // Sort alphabetically (ascending - A to Z)
                    if (entryA_name.compare(entryB_name) < 0)
                        return true;
                    break;
                
                case 1: // Sort alphabetically (descending - Z to A)
                    if (entryB_name.compare(entryA_name) < 0)
                        return true;
                    break;
                    
                case 2: // Sort by file size (largest first)
                    if (entryB.fileSize < entryA.fileSize)
                        return true;
                    break;
                
                case 3: // Sort by file size (smallest first)
                    if (entryA.fileSize < entryB.fileSize)
                        return true;
                    break;
            }
        }
        return false;
    }
    
    Result GetDirList(const std::string &path, std::vector<FS_DirectoryEntry> &entries) {
        if (!entries.empty())
            entries.clear();
            
        Result ret = 0;
        Handle dir = 0;
        std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());
        
        if (R_FAILED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_UTF16, path_u16.c_str())))) {
            Log::Error("FSUSER_OpenDirectory(%s) failed: 0x%x\n", path.c_str(), ret);
            return ret;
        }
        
        u32 entry_count = 0;
        
        do {
            FS_DirectoryEntry entry;
            
            if (R_FAILED(ret = FSDIR_Read(dir, &entry_count, 1, &entry))) {
                Log::Error("FSDIR_Read(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
            
            if (entry_count == 1)
                entries.push_back(entry);
        } while(entry_count > 0);
        
        std::sort(entries.begin(), entries.end(), FS::Sort);
        
        if (R_FAILED(ret = FSDIR_Close(dir))) {
            Log::Error("FSDIR_Close(%s) failed: 0x%x\n", path.c_str(), ret);
            return ret;
        }
        
        return 0;
    }
    
    static Result ChangeDir(const std::string &path, std::vector<FS_DirectoryEntry> &entries) {
        Result ret = 0;
        std::vector<FS_DirectoryEntry> new_entries;
        
        if (R_FAILED(ret = FS::GetDirList(path, new_entries)))
            return ret;
            
        entries.clear();
        cfg.cwd = path;

        if (archive == sdmc_archive)
            Config::Save(cfg);
        
        entries = new_entries;
        return 0;
    }
    
    Result ChangeDirNext(const std::string &path, std::vector<FS_DirectoryEntry> &entries) {
        std::string new_path = cfg.cwd;
        new_path.append(path);
        new_path.append("/");
        return FS::ChangeDir(new_path, entries);
    }
    
    Result ChangeDirPrev(std::vector<FS_DirectoryEntry> &entries) {
        std::filesystem::path path = (cfg.cwd.length() <= 1)? cfg.cwd : cfg.cwd.substr(0, cfg.cwd.size() - 1);
        std::string parent_path = path.parent_path();
        return FS::ChangeDir((parent_path.length() <= 1)? parent_path : parent_path.append("/"), entries);
    }
    
    Result Delete(FS_DirectoryEntry *entry) {
        Result ret = 0;
        
        std::u16string cwd = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(cfg.cwd.data());
        std::u16string path = cwd;
        path.append(reinterpret_cast<const char16_t *>(entry->name));
        
        if (entry->attributes & FS_ATTRIBUTE_DIRECTORY) {
            if (R_FAILED(ret = FSUSER_DeleteDirectoryRecursively(archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                Log::Error("FSUSER_DeleteDirectoryRecursively(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
        }
        else {
            if (R_FAILED(ret = FSUSER_DeleteFile(archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                Log::Error("FSUSER_DeleteFile(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
        }
        
        return 0;
    }
    
    Result Rename(FS_DirectoryEntry *entry, const std::string &filename) {
        Result ret = 0;
        std::u16string cwd = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(cfg.cwd.data());
        std::u16string filename_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(filename.data());
        
        std::u16string path = cwd;
        path.append(reinterpret_cast<const char16_t *>(entry->name));
        
        std::u16string new_path = cwd;
        new_path.append(filename_u16);
        
        if (entry->attributes & FS_ATTRIBUTE_DIRECTORY) {
            if (R_FAILED(ret = FSUSER_RenameDirectory(archive, fsMakePath(PATH_UTF16, path.c_str()), archive, fsMakePath(PATH_UTF16, new_path.c_str())))) {
                Log::Error("FSUSER_RenameDirectory(%s, %s) failed: 0x%x\n", path.c_str(), new_path.c_str(), ret);
                return ret;
            }
        }
        else {
            if (R_FAILED(ret = FSUSER_RenameFile(archive, fsMakePath(PATH_UTF16, path.c_str()), archive, fsMakePath(PATH_UTF16, new_path.c_str())))) {
                Log::Error("FSUSER_RenameFile(%s, %s) failed: 0x%x\n", path.c_str(), new_path.c_str(), ret);
                return ret;
            }
        }
        
        return 0;
    }
    
    static Result CopyFile(const std::u16string &src_path, const std::u16string &dest_path) {
        Result ret = 0;
        Handle src_handle, dest_handle;
        
        if (R_FAILED(ret = FSUSER_OpenFile(&src_handle, src_archive, fsMakePath(PATH_UTF16, src_path.c_str()), FS_OPEN_READ, 0))) {
            Log::Error("FSUSER_OpenFile(%s) failed: 0x%x\n", src_path.c_str(), ret);
            return ret;
        }
        
        u64 size = 0;
        if (R_FAILED(ret = FSFILE_GetSize(src_handle, &size))) {
            Log::Error("FSFILE_GetSize(%s) failed: 0x%x\n", src_path.c_str(), ret);
            FSFILE_Close(src_handle);
            return ret;
        }

        // Make sure we have enough storage to carry out this operation
        if (FS::GetFreeStorage(src_archive == sdmc_archive? SYSTEM_MEDIATYPE_SD : SYSTEM_MEDIATYPE_CTR_NAND) < size) {
            Log::Error("Not enough storage is available to process this command 0x%x\n", src_path.c_str(), ret);
            FSFILE_Close(src_handle);
            return -1;
        }
        
        // This may fail or not, but we don't care -> create the file if it doesn't exist, otherwise continue.
        FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, dest_path.c_str()), 0, size);
        
        if (R_FAILED(ret = FSUSER_OpenFile(&dest_handle, archive, fsMakePath(PATH_UTF16, dest_path.c_str()), FS_OPEN_WRITE, 0))) {
            Log::Error("FSUSER_OpenFile(%s) failed: 0x%x\n", dest_path.c_str(), ret);
            FSFILE_Close(src_handle);
            return ret;
        }
        
        u32 bytes_read = 0, bytes_written = 0;
        const u64 buf_size = 0x10000;
        u64 offset = 0;
        u8 *buf = new u8[buf_size];
        std::string filename = std::filesystem::path(std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(src_path.data())).filename();
        
        do {
            std::memset(buf, 0, buf_size);
            
            if (R_FAILED(ret = FSFILE_Read(src_handle, &bytes_read, offset, buf, buf_size))) {
                Log::Error("FSFILE_Read(%s) failed: 0x%x\n", src_path.c_str(), ret);
                delete[] buf;
                FSFILE_Close(src_handle);
                FSFILE_Close(dest_handle);
                return ret;
            }
            
            if (R_FAILED(ret = FSFILE_Write(dest_handle, &bytes_written, offset, buf, bytes_read, FS_WRITE_FLUSH))) {
                Log::Error("FSFILE_Write(%s) failed: 0x%x\n", dest_path.c_str(), ret);
                delete[] buf;
                FSFILE_Close(src_handle);
                FSFILE_Close(dest_handle);
                return ret;
            }
            
            offset += bytes_read;
            GUI::ProgressBar("Copying", filename.c_str(), static_cast<float>(offset), static_cast<float>(size));
        } while(offset < size);
        
        delete[] buf;
        FSFILE_Close(src_handle);
        FSFILE_Close(dest_handle);
        return 0;
    }
    
    static Result CopyDir(const std::u16string &src_path, const std::u16string &dest_path) {
        Result ret = 0;
        Handle dir;
        
        if (R_FAILED(ret = FSUSER_OpenDirectory(&dir, src_archive, fsMakePath(PATH_UTF16, src_path.c_str())))) {
            Log::Error("FSUSER_OpenDirectory(%s) failed: 0x%x\n", src_path, ret);
            return ret;
        }
        
        // This may fail or not, but we don't care -> make the dir if it doesn't exist, otherwise continue.
        FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, dest_path.c_str()), 0);
        
        u32 entry_count = 0;
        
        do {
            FS_DirectoryEntry entry;
            
            if (R_FAILED(ret = FSDIR_Read(dir, &entry_count, 1, &entry))) {
                Log::Error("FSDIR_Read(%s) failed: 0x%x\n", src_path.c_str(), ret);
                return ret;
            }
            
            if (entry_count == 1) {
                std::u16string src = src_path;
                src.append(u"/");
                src.append(reinterpret_cast<const char16_t *>(entry.name));
                
                std::u16string dest = dest_path;
                dest.append(u"/");
                dest.append(reinterpret_cast<const char16_t *>(entry.name));
                
                if (entry.attributes & FS_ATTRIBUTE_DIRECTORY)
                    FS::CopyDir(src, dest); // Copy Folder (via recursion)
                else
                    FS::CopyFile(src, dest); // Copy File
            }
        } while(entry_count > 0);
        
        if (R_FAILED(ret = FSDIR_Close(dir))) {
            Log::Error("FSDIR_Close(%s) failed: 0x%x\n", src_path.c_str(), ret);
            return ret;
        }
        
        return 0;
    }
    
    void Copy(FS_DirectoryEntry *entry, const std::string &path) {
        fs_copy_entry.copy_path = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());
        fs_copy_entry.copy_path.append(reinterpret_cast<const char16_t *>(entry->name));
        fs_copy_entry.copy_filename.append(reinterpret_cast<const char16_t *>(entry->name));
        
        if (entry->attributes & FS_ATTRIBUTE_DIRECTORY)
            fs_copy_entry.is_dir = true;
            
        src_archive = archive;
    }
    
    Result Paste(void) {
        Result ret = 0;
        std::u16string path = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(cfg.cwd.data());
        path.append(fs_copy_entry.copy_filename);
        
        if (fs_copy_entry.is_dir) // Copy folder recursively
            ret = FS::CopyDir(fs_copy_entry.copy_path, path);
        else // Copy file
            ret = FS::CopyFile(fs_copy_entry.copy_path, path);
            
        fs_copy_entry.copy_path.clear();
        fs_copy_entry.copy_filename.clear();
        fs_copy_entry.is_dir = false;
        return ret;
    }
    
    Result Move(void) {
        Result ret = 0;
        std::u16string path = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(cfg.cwd.data());
        path.append(fs_copy_entry.copy_filename);
        
        if (fs_copy_entry.is_dir) {
            if (R_FAILED(ret = FSUSER_RenameDirectory(src_archive, fsMakePath(PATH_UTF16, fs_copy_entry.copy_path.c_str()), archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                Log::Error("FSUSER_RenameDirectory(%s, %s) failed: 0x%x\n", path.c_str(), fs_copy_entry.copy_filename.c_str(), ret);
                return ret;
            }
        }
        else {
            if (R_FAILED(ret = FSUSER_RenameFile(src_archive, fsMakePath(PATH_UTF16, fs_copy_entry.copy_path.c_str()), archive, fsMakePath(PATH_UTF16, path.c_str())))) {
                Log::Error("FSUSER_RenameFile(%s, %s) failed: 0x%x\n", path.c_str(), fs_copy_entry.copy_filename.c_str(), ret);
                return ret;
            }
        }
        
        fs_copy_entry.copy_path.clear();
        fs_copy_entry.copy_filename.clear();
        fs_copy_entry.is_dir = false;
        return 0;
    }
}
