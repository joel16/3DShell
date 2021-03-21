#include <archive.h>
#include <archive_entry.h>
#include <cstring>
#include <filesystem>
#include <string>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "utils.h"

namespace ArchiveHelper {
    u64 CountFiles(const std::string &path) {
        int ret = 0;
        u64 count = 0;

        struct archive *arch = archive_read_new();
        archive_read_support_format_all(arch);

        if ((ret = archive_read_open_filename(arch, path.c_str(), 0x3000)) != ARCHIVE_OK) {
            archive_read_close(arch);
            archive_read_free(arch);
            return ret;
        }

        struct archive_entry *entry;
        while((ret = archive_read_next_header(arch, &entry)) == ARCHIVE_OK) {
            if (ret == ARCHIVE_EOF)
                break;

            count++;
        }
        
        archive_read_close(arch);
        archive_read_free(arch);
        return count;
    }

    int Extract(const std::string &path) {
        int ret = 0;

        int flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;

        struct archive *arch = archive_read_new();
        archive_read_support_format_all(arch);

        struct archive *ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, flags);

        if ((ret = archive_read_open_filename(arch, path.c_str(), 0x3000)) != ARCHIVE_OK) {
            archive_read_close(arch);
            archive_read_free(arch);
            archive_write_close(ext);
            archive_write_free(ext);
            Log::Error("archive_read_open_filename(%s) failed: %s\n", path.c_str(), archive_error_string(arch));
            return ret;
        }

        u64 index = 0, count = ArchiveHelper::CountFiles(path);
        std::string filename = std::filesystem::path(path).filename();
        std::string dest = cfg.cwd;
        dest.append(std::filesystem::path(path).stem());
        FSUSER_CreateDirectory(archive, fsMakePath(PATH_ASCII, dest.c_str()), 0);

        struct archive_entry *entry = nullptr;
        while((ret = archive_read_next_header(arch, &entry)) == ARCHIVE_OK) {
            if (Utils::IsCancelButtonPressed()) {
                archive_read_close(arch);
                archive_read_free(arch);
                archive_write_close(ext);
                archive_write_free(ext);
                return 0;
            }

            if (ret == ARCHIVE_EOF)
                break;
            if (ret < ARCHIVE_OK)
                Log::Error("archive_read_next_header(%s) failed: %s\n", path.c_str(), archive_error_string(arch));
            
            const char *entry_name = archive_entry_pathname(entry);
            std::string dest_path = dest + "/";
            dest_path.append(entry_name);
            
            archive_entry_update_pathname_utf8(entry, dest_path.c_str());
            
            s64 entry_size = archive_entry_size(entry);
            ret = archive_write_header(ext, entry);
            if (ret < ARCHIVE_OK)
                Log::Error("archive_write_header(%s) failed: %s\n", dest_path.c_str(), archive_error_string(arch));
            else if (entry_size > 0) {
                Handle dest_handle;
                FSUSER_CreateFile(archive, fsMakePath(PATH_ASCII, dest_path.c_str()), 0, entry_size);
                
                if (R_FAILED(ret = FSUSER_OpenFile(&dest_handle, archive, fsMakePath(PATH_ASCII, dest_path.c_str()), FS_OPEN_WRITE, 0))) {
                    Log::Error("FSUSER_OpenFile(%s) failed: 0x%x\n", dest_path.c_str(), ret);
                    archive_read_close(arch);
                    archive_read_free(arch);
                    archive_write_close(ext);
                    archive_write_free(ext);
                    return ret;
                }
                
                u32 bytes_written = 0;
                const u64 buf_size = 0x10000;
                u64 offset = 0;
                u8 *buf = new u8[buf_size];
                
                do {
                    if (Utils::IsCancelButtonPressed()) {
                        FSFILE_SetSize(dest_handle, offset);
                        archive_read_close(arch);
                        archive_read_free(arch);
                        archive_write_close(ext);
                        archive_write_free(ext);
                        delete[] buf;
                        FSFILE_Close(dest_handle);
                        return 0;
                    }

                    std::memset(buf, 0, buf_size);
                    u32 bytes_read = archive_read_data(arch, buf, buf_size);
                    
                    if (R_FAILED(ret = FSFILE_Write(dest_handle, &bytes_written, offset, buf, bytes_read, FS_WRITE_FLUSH))) {
                        Log::Error("FSFILE_Write(%s) failed: 0x%x\n", dest_path.c_str(), ret);
                        archive_read_close(arch);
                        archive_read_free(arch);
                        archive_write_close(ext);
                        archive_write_free(ext);
                        delete[] buf;
                        FSFILE_Close(dest_handle);
                        return ret;
                    }
                    
                    offset += bytes_read;
                } while(offset < static_cast<u64>(entry_size));

                delete[] buf;
                FSFILE_Close(dest_handle);
            }

            ret = archive_write_finish_entry(ext);
            if (ret < ARCHIVE_OK)
                Log::Error("archive_write_finish_entry(%s) failed: %s\n", path.c_str(), archive_error_string(arch));

            GUI::ProgressBar("Extracting", filename, static_cast<float>(index), static_cast<float>(count));
            index++;
        }
        
        archive_read_close(arch);
        archive_read_free(arch);
        archive_write_close(ext);
        archive_write_free(ext);
        return 0;
    }
}
