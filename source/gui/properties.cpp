#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {
    static float okHeight = 0.f, okWidth = 0.f;

    void DisplayProperties(GuiData& data) {
        GUI::DrawImage(propertiesDialog[cfg.theme], ((320 - (propertiesDialog[0].subtex->width)) / 2), ((240 - (propertiesDialog[0].subtex->height)) / 2) + 10);
        GUI::DrawText(((320 - (propertiesDialog[0].subtex->width)) / 2) + 6, ((240 - (propertiesDialog[0].subtex->height)) / 2) + 13, 0.42f, guiTitleColour[cfg.theme], "Properties");

        GUI::DrawTextf(66, 57, 0.42f, guiTextColour[cfg.theme],
            cfg.cwd.length() > 22? "Parent: %.22s..." : "Parent: %s", cfg.cwd.c_str());
        if (!(data.entries[data.selected].attributes & FS_ATTRIBUTE_DIRECTORY)) {
            char size[16];
            Utils::GetSizeString(size, static_cast<double>(data.entries[data.selected].fileSize));
            GUI::DrawTextf(66, 73, 0.42f, guiTextColour[cfg.theme], "Size: %s", size);
        }

        GUI::GetTextDimensions(0.42f, &okWidth, &okHeight, "OK");
        GUI::DrawRect((253 - okWidth) - 5, (218 - okHeight) - 5, okWidth + 10, okHeight + 10, guiSelectorColour[cfg.theme]);

        GUI::DrawText(253 - okWidth, (218 - okHeight), 0.42f, guiTitleColour[cfg.theme], "OK");
    }

    void ControlProperties(GuiData& data, u32& kDown) {
        if (kDown & KEY_A) {
            data.state = GUI_STATE_OPTIONS;
        }
        else if (kDown & KEY_B) {
            data.state = GUI_STATE_OPTIONS;
        }

        if (Touch::Rect((253 - okWidth) - 5, (218 - okHeight) - 5, ((253 - okWidth) - 5) + okWidth + 10, ((218 - okHeight) - 5) + okHeight + 10)) {
            if (kDown & KEY_TOUCH) {
                data.state = GUI_STATE_OPTIONS;
            }
        }
    }
}
