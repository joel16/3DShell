#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    static float ok_height = 0.f, ok_width = 0.f;

    void DisplayProperties(MenuItem *item) {
        C2D::Image(cfg.dark_theme? properties_dialog_dark : properties_dialog, ((320 - (properties_dialog.subtex->width)) / 2), ((240 - (properties_dialog.subtex->height)) / 2) + 10);
        C2D::Text(((320 - (properties_dialog.subtex->width)) / 2) + 6, ((240 - (properties_dialog.subtex->height)) / 2) + 13, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Properties");

        C2D::Textf(66, 57, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
            cfg.cwd.length() > 22? "Parent: %.22s..." : "Parent: %s", cfg.cwd.c_str());
        if (!(item->entries[item->selected].attributes & FS_ATTRIBUTE_DIRECTORY)) {
            char utils_size[16];
            Utils::GetSizeString(utils_size, static_cast<double>(item->entries[item->selected].fileSize));
            C2D::Textf(66, 73, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Size: %s", utils_size);
        }

        C2D::GetTextSize(0.42f, &ok_width, &ok_height, "OK");
        C2D::Rect((253 - ok_width) - 5, (218 - ok_height) - 5, ok_width + 10, ok_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

        C2D::Text(253 - ok_width, (218 - ok_height), 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");
    }

    void ControlProperties(MenuItem *item, u32 *kDown) {
        if (*kDown & KEY_A)
            item->state = MENU_STATE_OPTIONS;
        else if (*kDown & KEY_B)
            item->state = MENU_STATE_OPTIONS;
    }
}
