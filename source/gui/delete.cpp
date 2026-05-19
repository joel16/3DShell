#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {
    static int selection = 0;
    constexpr const char *prompt = "Do you wish to continue?";
    static float cancelHeight = 0.0f, cancelWidth = 0.0f, confirmHeight = 0.0f, confirmWidth = 0.0f, promptWidth = 0.0f;

    static void Delete(GuiData& data, int& selection) {
        Result ret = 0;
        Log::Close();
        
        if ((data.checkedCount > 1) && (!data.checkedCwd.compare(cfg.cwd))) {
            for (u32 i = 0; i < data.checked.size(); i++) {
                if (data.checked.at(i)) {
                    if (R_FAILED(ret = FS::Delete(data.entries[i]))) {
                        FS::GetDirList(cfg.cwd, data.entries);
                        GUI::ResetCheckbox(data);
                        break;
                    }
                }
            }
        }
        else
            ret = FS::Delete(data.entries[data.selected]);
        
        if (R_SUCCEEDED(ret)) {
            FS::GetDirList(cfg.cwd, data.entries);
            GUI::ResetCheckbox(data);
        }
        
        GUI::LoadStorageBar(data);
        Log::Open();
        selection = 0;
        data.selected = 0;
        data.state = GUI_STATE_FILEBROWSER;
    }

    void DisplayDeleteOptions(GuiData& data) {
        GUI::DrawImage(dialog[cfg.theme], ((320 - (dialog[0].subtex->width)) / 2), ((240 - (dialog[0].subtex->height)) / 2));
        GUI::DrawText(((320 - (dialog[0].subtex->width)) / 2) + 6, ((240 - (dialog[0].subtex->height)) / 2) + 6 - 3, 0.42f, guiTitleColour[cfg.theme], "Delete");

        GUI::GetTextDimensions(0.42f, &promptWidth, nullptr, prompt);
        GUI::DrawText(((320 - (promptWidth)) / 2), ((240 - (dialog[0].subtex->height)) / 2) + 40 - 3, 0.42f, guiTextColour[cfg.theme], prompt);
        
        GUI::GetTextDimensions(0.42f, &confirmWidth, &confirmHeight, "YES");
        GUI::GetTextDimensions(0.42f, &cancelWidth, &cancelHeight, "NO");
        
        if (selection == 0) {
            GUI::DrawRect((288 - cancelWidth) - 5, (159 - cancelHeight) - 5, cancelWidth + 10, cancelHeight + 10, guiSelectorColour[cfg.theme]);
        }
        else {
            GUI::DrawRect((248 - (confirmWidth)) - 5, (159 - confirmHeight) - 5, confirmWidth + 10, confirmHeight + 10, guiSelectorColour[cfg.theme]);
        }
            
        GUI::DrawText(248 - (confirmWidth), (159 - confirmHeight) - 3, 0.42f, guiTitleColour[cfg.theme], "YES");
        GUI::DrawText(288 - cancelWidth, (159 - cancelHeight) - 3, 0.42f, guiTitleColour[cfg.theme], "NO");
    }

    void ControlDeleteOptions(GuiData& data, u32& kDown) {
        if (kDown & KEY_RIGHT) {
            selection++;
        }
        else if (kDown & KEY_LEFT) {
            selection--;
        }

        if (kDown & KEY_A) {
            if (selection == 1) {
                GUI::Delete(data, selection);
            }
            else {
                data.state = GUI_STATE_OPTIONS;
            }

        }
        else if (kDown & KEY_B) {
            data.state = GUI_STATE_OPTIONS;
        }

        if (Touch::Rect((288 - cancelWidth) - 5, (159 - cancelHeight) - 5, ((288 - cancelWidth) - 5) + cancelWidth + 10, ((159 - cancelHeight) - 5) + cancelHeight + 10)) {
            selection = 0;
            
            if (kDown & KEY_TOUCH) {
                data.state = GUI_STATE_OPTIONS;
                selection = 0;
            }
        }
        else if (Touch::Rect((248 - (confirmWidth)) - 5, (159 - confirmHeight) - 5, ((248 - (confirmWidth)) - 5) + confirmWidth + 10, ((159 - confirmHeight) - 5) + confirmHeight + 10)) {
            selection = 1;
            
            if (kDown & KEY_TOUCH) {
                GUI::Delete(data, selection);
            }
        }
        
        Utils::Wrap(selection, 0, 1);
    }
}
