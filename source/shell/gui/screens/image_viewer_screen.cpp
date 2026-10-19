#include "image_viewer_screen.h"

#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "textures.h"
#include "utils.h"

namespace GUI {

ImageViewerScreen::ImageViewerScreen(MenuItem *item)
    : item_(item) {
    SetTopBackground(BLACK_BG);
    SetBottomBackground(BLACK_BG);
}

void ImageViewerScreen::OnFrame(u64 delta_time) {
    delta_time_ = delta_time;
}

void ImageViewerScreen::OnKeyHeld(u32 k_held) {
    k_held_ = k_held;
}

void ImageViewerScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    Control(k_down);
}

std::string ImageViewerScreen::EntryNameUtf8(u32 index) {
    std::u16string clean;
    for (std::size_t i = 0; i < (sizeof(item_->entries[index].name) / sizeof(u16)); i++) {
        u16 ch = item_->entries[index].name[i];
        if (!ch)
            break;
        if ((ch < 0xD800) || (ch > 0xDFFF))
            clean.push_back((char16_t)ch);
    }

    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(clean);
}

bool ImageViewerScreen::IsImageEntry(u32 index) {
    if (item_->entries[index].attributes & FS_ATTRIBUTE_DIRECTORY)
        return false;
    if (index >= item_->entries.size())
        return false;

    return (FS::GetFileType(EntryNameUtf8(index)) == FileTypeImage);
}

bool ImageViewerScreen::FindNeighborImage(int dir, u32 *out_index) {
    int count = static_cast<int>(item_->entries.size());
    if (count <= 1)
        return false;

    int sel = item_->selected;
    for (int step = 1; step < count; step++) {
        int i = ((sel + dir * step) % count + count) % count; /* wrap around */
        if (i == sel)
            break;
        if (IsImageEntry(static_cast<u32>(i))) {
            *out_index = static_cast<u32>(i);
            return true;
        }
    }

    return false;
}

bool ImageViewerScreen::SwitchImage(int dir) {
    u32 index = 0;
    if (!FindNeighborImage(dir, &index))
        return false;

    std::string path = cfg.cwd;
    path.append(EntryNameUtf8(index));

    C2D_Image new_texture;
    if (!Textures::LoadImageFile(path, &new_texture))
        return false;

    delete[] item_->texture.tex;
    delete[] item_->texture.subtex;
    item_->texture = new_texture;

    item_->selected = static_cast<int>(index); /* the browser resumes here */
    zoom_factor_ = 1.0f;
    pos_x_ = 0;
    pos_y_ = 0;
    return true;
}

bool ImageViewerScreen::DrawImage(C2D_Image image, float x, float y, float start, float end, float w, float h, float zoom_factor) {
    C2D_DrawParams params = {
        { x - (pos_x_ * zoom_factor - pos_x_) / 2, y - (pos_y_ * zoom_factor - pos_y_) / 2, w * zoom_factor, h * zoom_factor },
        { start, end },
        0.0f, 0.0f
    };

    return C2D_DrawImage(image, &params, nullptr);
}

void ImageViewerScreen::DrawImageTop() {
    if ((item_->texture.subtex->width == 432) && (item_->texture.subtex->height == 528))
        image_state_ = DIMENSION_NINTENDO_SCREENSHOT;
    else if ((item_->texture.subtex->width == 400) && ((item_->texture.subtex->height == 480) || (item_->texture.subtex->height == 482)))
        image_state_ = DIMENSION_DUAL_SCREEN;
    else {
        image_state_ = DIMENSION_DEFAULT;

        if (static_cast<float>(item_->texture.subtex->height) > 240.0f) {
            scale_factor_ = (240.0f / static_cast<float>(item_->texture.subtex->height));
            width_ = static_cast<float>(item_->texture.subtex->width) * scale_factor_;
            height_ = static_cast<float>(item_->texture.subtex->height) * scale_factor_;
        }
        else {
            width_ = static_cast<float>(item_->texture.subtex->width);
            height_ = static_cast<float>(item_->texture.subtex->height);
        }
    }

    switch (image_state_) {
        case DIMENSION_DEFAULT:
            DrawImage(item_->texture, ((400.0f - (width_ * zoom_factor_)) / 2), ((240.0f - (height_ * zoom_factor_)) / 2), 0, 0, width_, height_, zoom_factor_);
            break;

        case DIMENSION_NINTENDO_SCREENSHOT:
            DrawImage(item_->texture, 0, 0, 16, 16, item_->texture.subtex->width, item_->texture.subtex->height, 1.0f);
            break;

        case DIMENSION_DUAL_SCREEN:
            DrawImage(item_->texture, 0, 0, 0, 0, item_->texture.subtex->width, item_->texture.subtex->height, 1.0f);
            break;
    }
}

void ImageViewerScreen::DrawImageBottom() {
    switch (image_state_) {
        case DIMENSION_NINTENDO_SCREENSHOT:
            DrawImage(item_->texture, 0, 0, 56, 272, item_->texture.subtex->width, item_->texture.subtex->height, 1.0f);
            break;

        case DIMENSION_DUAL_SCREEN:
            DrawImage(item_->texture, 0, 0, 40, 240, item_->texture.subtex->width, item_->texture.subtex->height, 1.0f);
            break;

        default:
            break;
    }

    if (properties_) {
        C2D::Image(cfg.dark_theme? properties_dialog_dark : properties_dialog, ((320 - (properties_dialog.subtex->width)) / 2), ((240 - (properties_dialog.subtex->height)) / 2));
        C2D::Text(((320 - (properties_dialog.subtex->width)) / 2) + 6, ((240 - (properties_dialog.subtex->height)) / 2) + 6, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Properties");

        const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item_->entries[item_->selected].name);
        const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());
        C2D::Textf(66, 57, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Name: %.20s", filename.c_str());
        C2D::Textf(66, 73, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Width: %hu px", item_->texture.subtex->width);
        C2D::Textf(66, 89, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Height: %hu px", item_->texture.subtex->height);
        C2D::GetTextSize(0.42f, &ok_width_, &ok_height_, "OK");
        C2D::Rect((253 - ok_width_) - 5, (218 - ok_height_) - 15, ok_width_ + 10, ok_height_ + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        C2D::Text(253 - ok_width_, (218 - ok_height_) - 10, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");
    }
}

void ImageViewerScreen::DrawTop() {
    DrawImageTop();
}

void ImageViewerScreen::DrawBottom() {
    DrawImageBottom();
}

void ImageViewerScreen::Control(u32 k_down) {
    if (image_state_ == DIMENSION_DEFAULT) {
        if ((height_ * zoom_factor_ > 240) || (width_ * zoom_factor_ > 400)) {
            float velocity = 2 / zoom_factor_;

            if (k_held_ & KEY_CPAD_UP)
                pos_y_ -= ((velocity * zoom_factor_) * (delta_time_));
            else if (k_held_ & KEY_CPAD_DOWN)
                pos_y_ += ((velocity * zoom_factor_) * (delta_time_));
            else if (k_held_ & KEY_CPAD_LEFT)
                pos_x_ -= ((velocity * zoom_factor_) * (delta_time_));
            else if (k_held_ & KEY_CPAD_RIGHT)
                pos_x_ += ((velocity * zoom_factor_) * (delta_time_));
        }

        if ((k_held_ & KEY_DUP) || (k_held_ & KEY_CSTICK_UP)) {
            zoom_factor_ += 0.5f * ((delta_time_) * 0.001);

            if (zoom_factor_ > 2.0f)
                zoom_factor_ = 2.0f;
        }
        else if ((k_held_ & KEY_DDOWN) || (k_held_ & KEY_CSTICK_DOWN)) {
            zoom_factor_ -= 0.5f * ((delta_time_) * 0.001);

            if (zoom_factor_ < 0.5f)
                zoom_factor_ = 0.5f;

            if (zoom_factor_ <= 1.0f) {
                pos_x_ = 0;
                pos_y_ = 0;
            }
        }

        if (k_down & KEY_SELECT) { // Reset zoom/pos
            pos_x_ = 0;
            pos_y_ = 0;
            zoom_factor_ = 1.0f;
        }
    }

    if (k_down & KEY_A) {
        if (properties_)
            properties_ = false;
    }
    if (!properties_ && (k_down & (KEY_DLEFT | KEY_L))) {
        /* Previous image in the directory (wraps around). */
        SwitchImage(-1);
    }
    else if (!properties_ && (k_down & (KEY_DRIGHT | KEY_R))) {
        SwitchImage(1);
    }
    if (k_down & KEY_B) {
        if (!properties_) {
            delete[] item_->texture.tex;
            delete[] item_->texture.subtex;
            zoom_factor_ = 1.0f;
            pos_x_ = 0;
            pos_y_ = 0;
            UI::ScreenManager::Instance().Pop();
        }
        else
            properties_ = false;
    }
    else if (k_down & KEY_X) {
        if (!properties_)
            properties_ = true;
    }

    Utils::SetMax(&pos_x_, width_, width_);
    Utils::SetMin(&pos_x_, -width_, -width_);
    Utils::SetMax(&pos_y_, height_, height_);
    Utils::SetMin(&pos_y_, -height_, -height_);
}

}
