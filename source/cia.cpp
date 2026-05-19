#include <cstring>
#include <string>

#include "cia.h"
#include "fs.h"
#include "gui.h"
#include "log.h"

namespace CIA {
    constexpr const char *path = "/3ds/3DShell/3DShell_UPDATE.cia";

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
        u32 bytesRead = 0, bytesWritten = 0;
        u64 size = 0, offset = 0;
        Handle destHandle, srcHandle;
        AM_TitleEntry title;

        if (R_FAILED(ret = FSUSER_OpenFile(&srcHandle, sdmcArchive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0))) {
            Log::Error("FSUSER_OpenFile failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = AM_GetCiaFileInfo(MEDIATYPE_SD, &title, srcHandle))) {
            Log::Error("AM_GetCiaFileInfo failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = FSFILE_GetSize(srcHandle, &size))) {
            Log::Error("FSFILE_GetSize failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = AM_StartCiaInstall(MEDIATYPE_SD, &destHandle))) {
            Log::Error("AM_StartCiaInstall failed: 0x%x\n", ret);
            return ret;
        }
        
        u32 bufferSize = 0x10000;
        u8 *buffer = new u8[bufferSize];
        
        do {
            std::memset(buffer, 0, bufferSize);
            
            if (R_FAILED(ret = FSFILE_Read(srcHandle, &bytesRead, offset, buffer, bufferSize))) {
                delete[] buffer;
                FSFILE_Close(srcHandle);
                FSFILE_Close(destHandle);
                Log::Error("FSFILE_Read failed: 0x%x\n", ret);
                return ret;
            }
            
            if (R_FAILED(ret = FSFILE_Write(destHandle, &bytesWritten, offset, buffer, bytesRead, FS_WRITE_FLUSH))) {
                delete[] buffer;
                FSFILE_Close(srcHandle);
                FSFILE_Close(destHandle);
                Log::Error("FSFILE_Read failed: 0x%x\n", ret);
                return ret;
            }
            
            offset += bytesRead;
            //GUI::ProgressBar("Installing", "3DShell_UPDATE.cia", offset, size);
        } while(offset < size);
        
        if (bytesRead != bytesWritten) {
            AM_CancelCIAInstall(destHandle);
            delete[] buffer;
            Log::Error(".CIA bytes read and written mismatch: 0x%x\n", ret);
            return ret;
        }
        
        delete[] buffer;
        
        if (R_FAILED(ret = AM_FinishCiaInstall(destHandle))) {
            Log::Error("AM_FinishCiaInstall failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = FSFILE_Close(srcHandle))) {
            Log::Error("FSFILE_Close failed: 0x%x\n", ret);
            return ret;
        }
        
        FSUSER_DeleteFile(sdmcArchive, fsMakePath(PATH_ASCII, path)); // Delete update cia
        
        if (R_FAILED(ret = CIA::LaunchTitle(title.titleID))) {
            return ret;
        }
            
        return 0;
    }
}
