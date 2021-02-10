#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "cia.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "net.h"
#include "textures.h"
#include "utils.h"

// Kinda messy, perhaps clean this up later if I ever revisit this.
namespace GUI {
    static int selection = 0;
    static const std::string error = "Could not connect to network.";
    static const std::string prompt = "Do you wish to download and install vX.X.X?";
    static const std::string success = "Update installed. Please re-run the application";
    static const std::string no_updates = "You are already on the latest version";
    static float cancel_height = 0.f, cancel_width = 0.f, confirm_height = 0.f, confirm_width = 0.f, text_width = 0.f;
    static bool done = false;

    void DisplayUpdateOptions(bool *connection_status, bool *available, const std::string &tag) {
        C2D::Image(cfg.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

        if (!*connection_status) {
            C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Error");
            C2D::GetTextSize(0.42f, &text_width, nullptr, error.c_str());
            C2D::Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, error.c_str());
        }
        else if ((*connection_status) && (*available) && (!tag.empty()) && (!done)) {
            C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Update Available");
            C2D::GetTextSize(0.42f, &text_width, nullptr, prompt.c_str());
            C2D::Textf(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, 
                "Do you wish to download and install %s?", tag.c_str());
        }
        else if (!*available) {
            C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "No Updates");
            C2D::GetTextSize(0.42f, &text_width, nullptr, no_updates.c_str());
            C2D::Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, no_updates.c_str());
        }
        else if (done) {
            C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Update Successful");
            C2D::GetTextSize(0.42f, &text_width, nullptr, success.c_str());
            C2D::Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, success.c_str());
        }
        
        C2D::GetTextSize(0.42f, &confirm_width, &confirm_height, "YES");
        C2D::GetTextSize(0.42f, &cancel_width, &cancel_height, "NO");
        
        if (selection == 0)
            C2D::Rect((288 - cancel_width) - 5, (159 - cancel_height) - 5, cancel_width + 10, cancel_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        else if (selection == 1)
            C2D::Rect((248 - (confirm_width)) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            
        if ((!done) && (*connection_status) && (*available))
            C2D::Text(248 - (confirm_width), (159 - confirm_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
        
        C2D::Text(288 - cancel_width, (159 - cancel_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, ((!*connection_status) || (done) || (!*available))? "OK" : "NO");
    }

    void ControlUpdateOptions(MenuItem *item, u32 *kDown, bool *state, bool *connection_status, bool *available, const std::string &tag) {
        if (*kDown & KEY_RIGHT)
            selection++;
        else if (*kDown & KEY_LEFT)
            selection--;

        if (*kDown & KEY_A) {
            if (selection == 1) {
                Net::Init();
                Net::GetLatestRelease(tag);
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
            else if (!done) {
                *state = false;
                item->state = MENU_STATE_SETTINGS;
            }
            else if (done)
                longjmp(exit_jmp, 1);

        }
        else if (*kDown & KEY_B) {
            *state = false;
            item->state = MENU_STATE_SETTINGS;
        }

        if ((done) || (!*connection_status) || (!*available))
            Utils::SetBounds(&selection, 0, 0);
        else
            Utils::SetBounds(&selection, 0, 1);
    }
}
