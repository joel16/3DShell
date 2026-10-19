#include <3ds.h>

#include "c2d_helper.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

std::string __application_path__;

namespace Services {
    Result Init(void) {
        Result ret = 0;

        osSetSpeedupEnable(true);
        FS::OpenArchive(&sdmc_archive, ARCHIVE_SDMC);
        FS::OpenArchive(&nand_archive, ARCHIVE_NAND_CTR_FS);
        archive = sdmc_archive;
        Log::Open();
        Config::Load();
        
        if (R_FAILED(ret = acInit())) {
            Log::Error("acInit failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = amInit())) {
            Log::Error("amInit failed: 0x%x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = AM_QueryAvailableExternalTitleDatabase(nullptr))) {
            Log::Error("AM_QueryAvailableExternalTitleDatabase failed: 0x%x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = mcuHwcInit())) {
            Log::Error("mcuHwcInit failed: 0x%x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = ptmuInit())) {
            Log::Error("ptmuInit failed: 0x%x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = romfsInit())) {
            Log::Error("romfsInit failed: 0x%x\n", ret);
            return ret;
        }
        
        gfxInitDefault();
        C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
        C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
        C2D_Prepare();
        
        static_buf = C2D_TextBufNew(4096);
        dynamic_buf = C2D_TextBufNew(4096);
        size_buf = C2D_TextBufNew(4096);
        
        top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
        bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
        
        Textures::Init();
        romfsExit();
        return 0;
    }

    void Exit(void) {
        Textures::Exit();
        C2D_TextBufDelete(size_buf);
        C2D_TextBufDelete(dynamic_buf);
        C2D_TextBufDelete(static_buf);
        C2D_Fini();
        C3D_Fini();
        gfxExit();
        ptmuExit();
        mcuHwcExit();
        amExit();
        acExit();
        FS::CloseArchive(nand_archive);
        FS::CloseArchive(sdmc_archive);
    }
}

int main(int argc, char* argv[]) {
    if (envIsHomebrew())  {
        __application_path__ = argv[0];
        __application_path__.erase(0, 5);
    }

    Services::Init();
    GUI::Loop();
    Services::Exit();
    return 0;
}
