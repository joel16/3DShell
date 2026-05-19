#include "cia.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "net.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static float cancelHeight = 0.f, cancelWidth = 0.f, confirmHeight = 0.f, confirmWidth = 0.f, textWidth = 0.f;

// Kinda messy, perhaps clean this up later if I ever revisit this.
namespace GUI {
    static int selection = 0;
    constexpr const char *error = "Could not connect to network.";
    constexpr const char *prompt = "Do you wish to download and install vX.X.X?";
    constexpr const char *success = "Update installed. Please re-run the application";
    constexpr const char *noUpdates = "You are already on the latest version";
    static bool done = false;

    void DownloadHelper(const std::string &tag) {
        Net::Init();
        s32 prio = 0;
        downloadProgress = true;
        svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
        Thread thread = threadCreate(GUI::DownloadProgressBar, nullptr, 32 * 1024, prio - 1, -2, false);
        Net::GetLatestRelease(tag);
        downloadProgress = false;
        threadJoin(thread, U64_MAX);
        threadFree(thread);
        Net::Exit();
        
        if (envIsHomebrew()) {
            Result ret = 0;
            if (R_FAILED(ret = FSUSER_DeleteFile(sdmcArchive, fsMakePath(PATH_ASCII, __application_path__.c_str())))) {
                Log::Error("FSUSER_DeleteFile(%s) failed: 0x%x\n", __application_path__.c_str(), ret);
            }
                
            if (R_FAILED(ret = FSUSER_RenameFile(sdmcArchive, fsMakePath(PATH_ASCII, "/3ds/3DShell/3DShell_UPDATE.3dsx"), sdmcArchive, fsMakePath(PATH_ASCII, __application_path__.c_str())))) {
                Log::Error("FSUSER_RenameFile(update) failed: 0x%x\n", ret);
            }
        }
        else {
            CIA::InstallUpdate();
        }
        
        done = true;
    }

    void DisplayUpdateOptions(bool& connectionStatus, bool& available, const std::string& tag) {
        GUI::DrawImage(dialog[cfg.theme], ((320 - (dialog[0].subtex->width)) / 2), ((240 - (dialog[0].subtex->height)) / 2));

        if (!connectionStatus) {
            GUI::DrawText(((320 - (dialog[0].subtex->width)) / 2) + 6, ((240 - (dialog[0].subtex->height)) / 2) + 6 - 3, 0.42f, guiTitleColour[cfg.theme], "Error");
            GUI::GetTextDimensions(0.42f, &textWidth, nullptr, error);
            GUI::DrawText(((320 - (textWidth)) / 2), ((240 - (dialog[0].subtex->height)) / 2) + 40 - 3, 0.42f, guiTextColour[cfg.theme], error);
        }
        else if ((connectionStatus) && (available) && (!tag.empty()) && (!done)) {
            GUI::DrawText(((320 - (dialog[0].subtex->width)) / 2) + 6, ((240 - (dialog[0].subtex->height)) / 2) + 6 - 3, 0.42f, guiTitleColour[cfg.theme], "Update Available");
            GUI::GetTextDimensions(0.42f, &textWidth, nullptr, prompt);
            GUI::DrawTextf(((320 - (textWidth)) / 2), ((240 - (dialog[0].subtex->height)) / 2) + 40 - 3, 0.42f, guiTextColour[cfg.theme], 
                "Do you wish to download and install %s?", tag.c_str());
        }
        else if (!available) {
            GUI::DrawText(((320 - (dialog[0].subtex->width)) / 2) + 6, ((240 - (dialog[0].subtex->height)) / 2) + 6 - 3, 0.42f, guiTitleColour[cfg.theme], "No Updates");
            GUI::GetTextDimensions(0.42f, &textWidth, nullptr, noUpdates);
            GUI::DrawText(((320 - (textWidth)) / 2), ((240 - (dialog[0].subtex->height)) / 2) + 40 - 3, 0.42f, guiTextColour[cfg.theme], noUpdates);
        }
        else if (done) {
            GUI::DrawText(((320 - (dialog[0].subtex->width)) / 2) + 6, ((240 - (dialog[0].subtex->height)) / 2) + 6 - 3, 0.42f, guiTitleColour[cfg.theme], "Update Successful");
            GUI::GetTextDimensions(0.42f, &textWidth, nullptr, success);
            GUI::DrawText(((320 - (textWidth)) / 2), ((240 - (dialog[0].subtex->height)) / 2) + 40 - 3, 0.42f, guiTextColour[cfg.theme], success);
        }
        
        GUI::GetTextDimensions(0.42f, &confirmWidth, &confirmHeight, "YES");
        GUI::GetTextDimensions(0.42f, &cancelWidth, &cancelHeight, "NO");
        
        if (selection == 0) {
            GUI::DrawRect((288 - cancelWidth) - 5, (159 - cancelHeight) - 5, cancelWidth + 10, cancelHeight + 10, guiSelectorColour[cfg.theme]);
        }
        else if (selection == 1) {
            GUI::DrawRect((248 - (confirmWidth)) - 5, (159 - confirmHeight) - 5, confirmWidth + 10, confirmHeight + 10, guiSelectorColour[cfg.theme]);
        }
            
        if ((!done) && (connectionStatus) && (available)) {
            GUI::DrawText(248 - (confirmWidth), (159 - confirmHeight) - 3, 0.42f, guiTitleColour[cfg.theme], "YES");
        }
        
        GUI::DrawText(288 - cancelWidth, (159 - cancelHeight) - 3, 0.42f, guiTitleColour[cfg.theme], ((!connectionStatus) || (done) || (!available))? "OK" : "NO");
    }

    void ControlUpdateOptions(GuiData& data, u32& kDown, bool& state, bool& connectionStatus, bool& available, const std::string &tag) {
        if (kDown & KEY_RIGHT) {
            selection++;
        }
        else if (kDown & KEY_LEFT) {
            selection--;
        }

        if (kDown & KEY_A) {
            if (selection == 1)
                GUI::DownloadHelper(tag);
            else if (!done) {
                state = false;
                data.state = GUI_STATE_SETTINGS;
            }
            else if (done) {
                longjmp(exitJmp, 1);
            }
        }
        else if (kDown & KEY_B) {
            state = false;
            data.state = GUI_STATE_SETTINGS;
        }

        if (Touch::Rect((288 - cancelWidth) - 5, (159 - cancelHeight) - 5, ((288 - cancelWidth) - 5) + cancelWidth + 10, ((159 - cancelHeight) - 5) + cancelHeight + 10)) {
            selection = 0;
            
            if (kDown & KEY_TOUCH) {
                if (!done) {
                    state = false;
                    data.state = GUI_STATE_SETTINGS;
                }
                else if (done) {
                    longjmp(exitJmp, 1);
                }
            }
        }
        else if (Touch::Rect((248 - (confirmWidth)) - 5, (159 - confirmHeight) - 5, ((248 - (confirmWidth)) - 5) + confirmWidth + 10, ((159 - confirmHeight) - 5) + confirmHeight + 10)) {
            selection = 1;
            
            if ((kDown & KEY_TOUCH) && (!done) && (connectionStatus) && (available)) {
                GUI::DownloadHelper(tag);
            }
        }

        if ((done) || (!connectionStatus) || (!available)) {
            Utils::Wrap(selection, 0, 0);
        }
        else {
            Utils::Wrap(selection, 0, 1);
        }
    }
}
