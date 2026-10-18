#include "cia.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "net.h"
#include "textures.h"
#include "utils.h"

static bool done = false;

namespace GUI {
    void DownloadHelper(const std::string &tag) {
        Net::Init();
        s32 prio = 0;
        download_progress = true;
        svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
        Thread thread = threadCreate(GUI::DownloadProgressBar, nullptr, 32 * 1024, prio - 1, -2, false);
        Net::GetLatestRelease(tag);
        download_progress = false;
        threadJoin(thread, U64_MAX);
        threadFree(thread);
        Net::Exit();

        if (envIsHomebrew()) {
            Result ret = 0;
            if (R_FAILED(ret = FSUSER_DeleteFile(sdmc_archive, fsMakePath(PATH_ASCII, __application_path__.c_str()))))
                Log::Error("FSUSER_DeleteFile(%s) failed: 0x%x\n", __application_path__, ret);

            if (R_FAILED(ret = FSUSER_RenameFile(sdmc_archive, fsMakePath(PATH_ASCII, "/3ds/3DShell/3DShell_UPDATE.3dsx"), sdmc_archive, fsMakePath(PATH_ASCII, __application_path__.c_str()))))
                Log::Error("FSUSER_RenameFile(update) failed: 0x%x\n", ret);
        }
        else
            CIA::InstallUpdate();

        done = true;
    }

    bool UpdateDone(void) {
        return done;
    }
}
