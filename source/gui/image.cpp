#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    enum IMAGE_STATES {
        DIMENSION_DEFAULT = 0,
        DIMENSION_NINTENDO_SCREENSHOT = 1,
        DIMENSION_DUAL_SCREEN = 2
    };

    static bool properties = false;
    static float ok_height = 0.f, ok_width = 0.f, scale_factor = 0.f, width = 0.f, height = 0.f, zoom_factor = 1.f;
    static int pos_x = 0, pos_y = 0;
    static IMAGE_STATES image_state = DIMENSION_DEFAULT;

    static bool DrawImage(C2D_Image image, float x, float y, float start, float end, float w, float h, float zoom_factor) {
        C2D_DrawParams params = {
            { x - (pos_x * zoom_factor - pos_x) / 2, y - (pos_y * zoom_factor - pos_y) / 2, w * zoom_factor, h * zoom_factor },
            { start, end },
            0.f, 0.f
        };
        
        return C2D_DrawImage(image, &params, nullptr);
    }

    void DisplayImageViewerTop(MenuItem *item) {
        C2D_TargetClear(top_screen, BLACK_BG);
        
        if ((item->texture.subtex->width == 432) && (item->texture.subtex->height == 528))
            image_state = DIMENSION_NINTENDO_SCREENSHOT;
        else if ((item->texture.subtex->width == 400) && ((item->texture.subtex->height == 480) || (item->texture.subtex->height == 482)))
            image_state = DIMENSION_DUAL_SCREEN;
        else {
            image_state = DIMENSION_DEFAULT;

            if (static_cast<float>(item->texture.subtex->height) > 240.f) {
                scale_factor = (240.f / static_cast<float>(item->texture.subtex->height));
                width = static_cast<float>(item->texture.subtex->width) * scale_factor;
                height = static_cast<float>(item->texture.subtex->height) * scale_factor;
            }
            else {
                width = static_cast<float>(item->texture.subtex->width);
                height = static_cast<float>(item->texture.subtex->height);
            }
        }

        switch (image_state) {
            case DIMENSION_DEFAULT:
                GUI::DrawImage(item->texture, ((400.f - (width * zoom_factor)) / 2), ((240.f - (height * zoom_factor)) / 2), 0, 0, width, height, zoom_factor);
                break;

            case DIMENSION_NINTENDO_SCREENSHOT:
                GUI::DrawImage(item->texture, 0, 0, 16, 16, item->texture.subtex->width, item->texture.subtex->height, 1.f);
                break;

            case DIMENSION_DUAL_SCREEN:
                GUI::DrawImage(item->texture, 0, 0, 0, 0, item->texture.subtex->width, item->texture.subtex->height, 1.f);
                break;
        }
    }

    void DisplayImageViewerBottom(MenuItem *item) {
        C2D_TargetClear(bottom_screen, BLACK_BG);

        switch (image_state) {
            case DIMENSION_NINTENDO_SCREENSHOT:
                GUI::DrawImage(item->texture, 0, 0, 56, 272, item->texture.subtex->width, item->texture.subtex->height, 1.f);
                break;

            case DIMENSION_DUAL_SCREEN:
                GUI::DrawImage(item->texture, 0, 0, 40, 240, item->texture.subtex->width, item->texture.subtex->height, 1.f);
                break;

            default:
                break;
        }

        if (properties) {
            C2D::Image(cfg.dark_theme? properties_dialog_dark : properties_dialog, ((320 - (properties_dialog.subtex->width)) / 2), ((240 - (properties_dialog.subtex->height)) / 2));
            C2D::Text(((320 - (properties_dialog.subtex->width)) / 2) + 6, ((240 - (properties_dialog.subtex->height)) / 2) + 6, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Properties");
            
            const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item->entries[item->selected].name);
            const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());
            C2D::Textf(66, 57, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Name: %.20s", filename.c_str());
            C2D::Textf(66, 73, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Width: %hu px", item->texture.subtex->width);
            C2D::Textf(66, 89, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Height: %hu px", item->texture.subtex->height);
            C2D::GetTextSize(0.42f, &ok_width, &ok_height, "OK");
            C2D::Rect((253 - ok_width) - 5, (218 - ok_height) - 15, ok_width + 10, ok_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            C2D::Text(253 - ok_width, (218 - ok_height) - 10, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");
        }
    }

    void ControlImageViewer(MenuItem *item, u32 *kDown, u32 *kHeld, u64 *delta_time) {
        if (image_state == DIMENSION_DEFAULT) {
            if ((height * zoom_factor > 240) || (width * zoom_factor > 400)) {
                float velocity = 2 / zoom_factor;
                
                if (*kHeld & KEY_CPAD_UP)
                    pos_y -= ((velocity * zoom_factor) * (*delta_time));
                else if (*kHeld & KEY_CPAD_DOWN)
                    pos_y += ((velocity * zoom_factor) * (*delta_time));
                else if (*kHeld & KEY_CPAD_LEFT)
                    pos_x -= ((velocity * zoom_factor) * (*delta_time));
                else if (*kHeld & KEY_CPAD_RIGHT)
                    pos_x += ((velocity * zoom_factor) * (*delta_time));
            }
            
            if ((*kHeld & KEY_DUP) || (*kHeld & KEY_CSTICK_UP)) {
                zoom_factor += 0.5f * ((*delta_time) * 0.001);
                
                if (zoom_factor > 2.f)
                    zoom_factor = 2.f;
            }
            else if ((*kHeld & KEY_DDOWN) || (*kHeld & KEY_CSTICK_DOWN)) {
                zoom_factor -= 0.5f * ((*delta_time) * 0.001);
                
                if (zoom_factor < 0.5f)
                    zoom_factor = 0.5f;
                    
                if (zoom_factor <= 1.f) {
                    pos_x = 0;
                    pos_y = 0;
                }
            }
            
            if (*kDown & KEY_SELECT) { // Reset zoom/pos
                pos_x = 0;
                pos_y = 0;
                zoom_factor = 1.f;
            }
        }
        
        if (*kDown & KEY_A) {
            if (properties)
                properties = false;
        }
        if (*kDown & KEY_B) {
            if (!properties) {
                delete[] item->texture.tex;
                delete[] item->texture.subtex;
                zoom_factor = 1.f;
                pos_x = 0;
                pos_y = 0;
                item->state = MENU_STATE_FILEBROWSER;
            }
            else
                properties = false;
        }
        else if (*kDown & KEY_X) {
            if (!properties)
                properties = true;
        }
        
        Utils::SetMax(&pos_x, width, width);
        Utils::SetMin(&pos_x, -width, -width);
        Utils::SetMax(&pos_y, height, height);
        Utils::SetMin(&pos_y, -height, -height);
    }
}
