#include <cstring>
#include <string>

#include "cia.h"
#include "fs.h"
#include "gui.h"
#include "log.h"

namespace CIA {
    static const std::string path = "/3ds/3DShell/3DShell_UPDATE.cia";

    static Result LaunchTitle(u64 titleId) {
        Result ret = 0;
        u8 param[0x300];
        u8 hmac[0x20];
        
        if (R_FAILED(ret = APT_PrepareToDoApplicationJump(0, titleId, MEDIATYPE_SD))) {
            Log::Error("APT_PrepareToDoApplicationJump failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = APT_DoApplicationJump(param, sizeof(param), hmac))) {
            Log::Error("APT_DoApplicationJump failed: 0x%x\n", ret);
            return ret;
        }
        
        return 0;
    }
    
    Result InstallUpdate(void) {
        Result ret = 0;
        u32 bytes_read = 0, bytes_written = 0;
        u64 size = 0, offset = 0;
        Handle dst_handle, src_handle;
        AM_TitleEntry title;

        if (R_FAILED(ret = FSUSER_OpenFile(&src_handle, sdmc_archive, fsMakePath(PATH_ASCII, path.c_str()), FS_OPEN_READ, 0))) {
            Log::Error("FSUSER_OpenFile failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = AM_GetCiaFileInfo(MEDIATYPE_SD, &title, src_handle))) {
            Log::Error("AM_GetCiaFileInfo failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = FSFILE_GetSize(src_handle, &size))) {
            Log::Error("FSFILE_GetSize failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = AM_StartCiaInstall(MEDIATYPE_SD, &dst_handle))) {
            Log::Error("AM_StartCiaInstall failed: 0x%x\n", ret);
            return ret;
        }
        
        u32 buffer_size = 0x10000;
        u8 *buffer = new u8[buffer_size];
        
        do {
            std::memset(buffer, 0, buffer_size);
            
            if (R_FAILED(ret = FSFILE_Read(src_handle, &bytes_read, offset, buffer, buffer_size))) {
                delete[] buffer;
                FSFILE_Close(src_handle);
                FSFILE_Close(dst_handle);
                Log::Error("FSFILE_Read failed: 0x%x\n", ret);
                return ret;
            }
            
            if (R_FAILED(ret = FSFILE_Write(dst_handle, &bytes_written, offset, buffer, bytes_read, FS_WRITE_FLUSH))) {
                delete[] buffer;
                FSFILE_Close(src_handle);
                FSFILE_Close(dst_handle);
                Log::Error("FSFILE_Read failed: 0x%x\n", ret);
                return ret;
            }
            
            offset += bytes_read;
            GUI::ProgressBar("Installing", "3DShell_UPDATE.cia", offset, size);
        } while(offset < size);
        
        if (bytes_read != bytes_written) {
            AM_CancelCIAInstall(dst_handle);
            delete[] buffer;
            Log::Error(".CIA bytes read and written mismatch: 0x%x\n", ret);
            return ret;
        }
        
        delete[] buffer;
        
        if (R_FAILED(ret = AM_FinishCiaInstall(dst_handle))) {
            Log::Error("AM_FinishCiaInstall failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = FSFILE_Close(src_handle))) {
            Log::Error("FSFILE_Close failed: 0x%x\n", ret);
            return ret;
        }
        
        FSUSER_DeleteFile(sdmc_archive, fsMakePath(PATH_ASCII, path.c_str())); // Delete update cia
        
        if (R_FAILED(ret = CIA::LaunchTitle(title.titleID)))
            return ret;
            
        return 0;
    }
}
