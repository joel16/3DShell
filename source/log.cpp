#include <cstdarg>
#include <string>

#include "config.h"
#include "fs.h"

namespace Log {
    static Handle handle;
    static u64 offset = 0;

    Result Open(void) {
        Result ret = 0;
        std::string path = "/3ds/3DShell/debug.log";

        // Delete existing logs on start up.
        if (FS::FileExists(sdmc_archive, path))
            FSUSER_DeleteFile(sdmc_archive, fsMakePath(PATH_ASCII, path.c_str()));

        if (!FS::FileExists(sdmc_archive, path)) {
            if (R_FAILED(ret = FSUSER_CreateFile(archive, fsMakePath(PATH_ASCII, path.c_str()), 0, 0)))
                return ret;
        }
        
        if (R_FAILED(ret = FSUSER_OpenFile(&handle, sdmc_archive, fsMakePath(PATH_ASCII, path.c_str()), FS_OPEN_WRITE, 0)))
            return ret;
            
        return 0;
    }
    
    Result Close(void) {
        Result ret = 0;
        
        if (R_FAILED(ret = FSFILE_Close(handle)))
            return ret;
            
        return 0;
    }

    void Error(const char *data, ...) {
        char buf[256];
        va_list args;
        va_start(args, data);
        std::vsnprintf(buf, sizeof(buf), data, args);
        va_end(args);
        
        std::string error_string = "[ERROR] ";
        error_string.append(buf);
        
        std::printf("%s", error_string.c_str());

        u32 bytes_written = 0;
        if (R_FAILED(FSFILE_Write(handle, &bytes_written, offset, error_string.data(), error_string.length(), FS_WRITE_FLUSH)))
            return;
            
        offset += bytes_written;
    }
}
