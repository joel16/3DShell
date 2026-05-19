#include <archive.h>
#include <archive_entry.h>
#include <filesystem>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

mode_t umask(mode_t mask) {
    return 0;
}

namespace ArchiveExtractor {
    static u64 CountFiles(const char *path) {
        int ret = 0;
        u64 count = 0;

        struct archive *arch = archive_read_new();
        archive_read_support_format_all(arch);

        if ((ret = archive_read_open_filename(arch, path, 0x3000)) != ARCHIVE_OK) {
            archive_read_close(arch);
            archive_read_free(arch);
            return ret;
        }

        struct archive_entry *entry;
        while((ret = archive_read_next_header(arch, &entry)) == ARCHIVE_OK) {
            count++;
        }
        
        archive_read_close(arch);
        archive_read_free(arch);
        return count;
    }

    static int CopyData(struct archive *src, struct archive *dest) {
        int ret = 0;
        const void *buff = nullptr;
        size_t size = 0;
        int64_t offset = 0;
        
        for (;;) {
            ret = archive_read_data_block(src, &buff, &size, &offset);
            if (ret == ARCHIVE_EOF) {
                return ARCHIVE_OK;
            }

            if (ret != ARCHIVE_OK) {
                return ret;
            }
                
            ret = archive_write_data_block(dest, buff, size, offset);
            if (ret != ARCHIVE_OK) {
                return ret;
            }
        }

        return 0;
    }

    static void FreeHandles(struct archive *read, struct archive *write) {
        archive_read_close(read);
        archive_read_free(read);
        archive_write_close(write);
        archive_write_free(write);
    }

    int Extract(const char *path) {
        int ret = 0;

        int flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;

        struct archive *arch = archive_read_new();
        archive_read_support_format_all(arch);

        struct archive *ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, flags);

        struct archive_entry *entry = nullptr;

        if ((ret = archive_read_open_filename(arch, path, 0x3000)) != ARCHIVE_OK) {
            Log::Error("archive_read_open_filename(%s) failed: %s\n", path, archive_error_string(arch));
            ArchiveExtractor::FreeHandles(arch, ext);
            return ret;
        }

        u64 index = 0, count = ArchiveExtractor::CountFiles(path);
        
        std::string filename = std::filesystem::path(path).filename();
        std::string filenameWithoutExt = std::filesystem::path(path).stem().string();

        std::u16string dest = cfg.cwd;
        dest.append(u"/");
        dest.append(Utils::UTF8ToUTF16(filenameWithoutExt.c_str())); // Append utf16 base filename
        
        FS::MakeDir(archive, dest);

        for (;;) {
            if (Utils::IsCancelButtonPressed()) {
                ArchiveExtractor::FreeHandles(arch, ext);
                return 0;
            }

            ret = archive_read_next_header(arch, &entry);
            if (ret == ARCHIVE_EOF) {
                break;
            }
                
            if (ret != ARCHIVE_OK) {
                Log::Error("archive_read_next_header(%s) failed: %s\n", path, archive_error_string(arch));
                ArchiveExtractor::FreeHandles(arch, ext);
                return ret;
            }
            
            const char *entryName = archive_entry_pathname(entry);
            std::string destPath = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(dest.c_str())) + "/";
            destPath.append(entryName);
            
            archive_entry_update_pathname_utf8(entry, destPath.c_str());
            ret = archive_write_header(ext, entry);
            
            if (ret < ARCHIVE_OK) {
                Log::Error("archive_write_header(%s) failed: %s\n", destPath.c_str(), archive_error_string(arch));
                ArchiveExtractor::FreeHandles(arch, ext);
                return ret;
            }
            else if (archive_entry_size(entry) > 0) {
                ret = ArchiveExtractor::CopyData(arch, ext);
                if (ret != ARCHIVE_OK) {
                    Log::Error("ArchiveExtractor::CopyData(%s) failed: %s\n", destPath.c_str(), archive_error_string(arch));
                    ArchiveExtractor::FreeHandles(arch, ext);
                    return ret;
                }
            }
            
            archive_write_finish_entry(ext);
            GUI::ProgressBar("Extracting", filename, index, count);
            index++;
        }
        
        ArchiveExtractor::FreeHandles(arch, ext);
        return 0;
    }
}
