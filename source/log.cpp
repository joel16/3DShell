#include <cstdarg>

#include "config.h"
#include "fs.h"

namespace Log {
    static Handle handle;
    static u64 offset = 0;

    Result Open(void) {
        Result ret = 0;
        std::u16string path = u"/3ds/3DShell/debug.log";

        // Delete existing logs on start up.
        if (FS::FileExists(sdmcArchive, path)) {
            FSUSER_DeleteFile(sdmcArchive, fsMakePath(PATH_UTF16, path.c_str()));
        }

        if (!FS::FileExists(sdmcArchive, path)) {
            if (R_FAILED(ret = FSUSER_CreateFile(sdmcArchive, fsMakePath(PATH_UTF16, path.c_str()), 0, 0))) {
                return ret;
            }
        }
        
        if (R_FAILED(ret = FSUSER_OpenFile(&handle, sdmcArchive, fsMakePath(PATH_UTF16, path.c_str()), FS_OPEN_WRITE, 0))) {
            return ret;
        }
            
        return 0;
    }
    
    Result Close(void) {
        Result ret = 0;
        
        if (R_FAILED(ret = FSFILE_Close(handle))) {
            return ret;
        }
            
        return 0;
    }

    void Error(const char *data, ...) {
        if (!cfg.debug) {
            return;
        }
        
        char buf[256];
        va_list args;
        va_start(args, data);
        std::vsnprintf(buf, sizeof(buf), data, args);
        va_end(args);
        
        std::string error_string = "[DEBUG] ";
        error_string.append(buf);
        
        std::printf("%s", error_string.c_str());

        u32 bytes_written = 0;
        if (R_FAILED(FSFILE_Write(handle, &bytes_written, offset, error_string.data(), error_string.length(), FS_WRITE_FLUSH))) {
            return;
        }
            
        offset += bytes_written;
    }
}
