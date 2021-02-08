#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    static bool properties = false;
    static float ok_height = 0.f, ok_width = 0.f;

    void DisplayImageViewerTop(MenuItem *item) {
        C2D_TargetClear(top_screen, BLACK_BG);
        C2D::Image(item->texture, ((400 - item->texture.subtex->width) / 2), ((240 - item->texture.subtex->height) / 2));
    }

    void DisplayImageViewerBottom(MenuItem *item) {
        C2D_TargetClear(bottom_screen, BLACK_BG);

        if (properties) {
            C2D::Image(cfg.dark_theme? properties_dialog_dark : properties_dialog, ((320 - (properties_dialog.subtex->width)) / 2), ((240 - (properties_dialog.subtex->height)) / 2));
            C2D::Text(((320 - (properties_dialog.subtex->width)) / 2) + 6, ((240 - (properties_dialog.subtex->height)) / 2) + 6, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Properties");
            
            const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item->entries[item->selected].name);
            const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());
            C2D::Textf(66, 57, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Name: %.20s", filename.c_str());
            C2D::Textf(66, 73, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Width: %d px", std::to_string(item->texture.subtex->width));
            C2D::Textf(66, 89, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Height: %d px", std::to_string(item->texture.subtex->height));
            C2D::GetTextSize(0.42f, &ok_width, &ok_height, "OK");
            C2D::Rect((253 - ok_width) - 5, (218 - ok_height) - 15, ok_width + 10, ok_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            C2D::Text(253 - ok_width, (218 - ok_height) - 10, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");
        }
    }

    void ControlImageViewer(MenuItem *item, u32 *kDown) {
        if (*kDown & KEY_A) {
            if (properties)
                properties = false;
        }
        if (*kDown & KEY_B) {
            if (!properties) {
                delete[] item->texture.tex;
                delete[] item->texture.subtex;
                item->state = MENU_STATE_FILEBROWSER;
            }
            else
                properties = false;
        }
        else if (*kDown & KEY_X) {
            if (!properties)
                properties = true;
        }
    }
}
