#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    static int selection = 0;
    static const std::string prompt = "Do you wish to continue?";
    static float cancel_height = 0.f, cancel_width = 0.f, confirm_height = 0.f, confirm_width = 0.f, prompt_width = 0.f;

    void DisplayDeleteOptions(MenuItem *item) {
        C2D::Image(cfg.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));
        C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Delete");

        C2D::GetTextSize(0.42f, &prompt_width, nullptr, prompt.c_str());
        C2D::Text(((320 - (prompt_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, prompt.c_str());
        
        C2D::GetTextSize(0.42f, &confirm_width, &confirm_height, "YES");
        C2D::GetTextSize(0.42f, &cancel_width, &cancel_height, "NO");
        
        if (selection == 0)
            C2D::Rect((288 - cancel_width) - 5, (159 - cancel_height) - 5, cancel_width + 10, cancel_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        else
            C2D::Rect((248 - (confirm_width)) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            
        C2D::Text(248 - (confirm_width), (159 - confirm_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
        C2D::Text(288 - cancel_width, (159 - cancel_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");
    }

    void ControlDeleteOptions(MenuItem *item, u32 *kDown) {
        if (*kDown & KEY_RIGHT)
            selection++;
        else if (*kDown & KEY_LEFT)
            selection--;

        if (*kDown & KEY_A) {
            if (selection == 1) {
                Result ret = 0;
                Log::Close();
                
                if ((item->checked_count > 1) && (!item->checked_cwd.compare(cfg.cwd))) {
                    for (u32 i = 0; i < item->checked.size(); i++) {
                        if (item->checked.at(i)) {
                            if (R_FAILED(ret = FS::Delete(&item->entries[i]))) {
                                FS::GetDirList(cfg.cwd, item->entries);
                                GUI::ResetCheckbox(item);
                                break;
                            }
                        }
                    }
                }
                else
                    ret = FS::Delete(&item->entries[item->selected]);
                    
                if (R_SUCCEEDED(ret)) {
                    FS::GetDirList(cfg.cwd, item->entries);
                    GUI::ResetCheckbox(item);
                }

                GUI::RecalcStorageSize(item);
                Log::Open();
                selection = 0;
                item->selected = 0;
                item->state = MENU_STATE_FILEBROWSER;
            }
            else
                item->state = MENU_STATE_OPTIONS;

        }
        else if (*kDown & KEY_B)
            item->state = MENU_STATE_OPTIONS;

        Utils::SetBounds(&selection, 0, 1);
    }
}
