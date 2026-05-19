#include <malloc.h>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"

std::string __application_path__;

int main(int argc, char* argv[]) {
    osSetSpeedupEnable(true);
    FS::OpenArchive(&sdmcArchive, ARCHIVE_SDMC);
    FS::OpenArchive(&nandArchive, ARCHIVE_NAND_CTR_FS);
    archive = sdmcArchive;
    Config::Load();
    Log::Open();

    if (envIsHomebrew())  {
        __application_path__ = argv[0];
        __application_path__.erase(0, 5);
    }
    
    // Real time services
#if !defined BUILD_CITRA
    mcuHwcInit();
#endif
    
    ptmuInit();
    socInit((u32*)memalign(0x1000, 0x10000), 0x10000);
    
    GUI::Init();
    GUI::MainMenu();
    GUI::Exit();
    
    socExit();
    ptmuExit();
#if !defined BUILD_CITRA
    mcuHwcExit();
#endif

    Log::Close();
    FS::CloseArchive(nandArchive);
    FS::CloseArchive(sdmcArchive);
    return 0;
}
