#ifndef _3D_SHELL_UI_DIALOG_H
#define _3D_SHELL_UI_DIALOG_H

#include <string>

#include "../container_control.h"
#include "../c2d_helper.h"

namespace UI {

class Dialog : public ContainerControl {
public:
    Dialog(const std::string &title, const Rect &bounds, u32 background, u32 title_bar_colour, u32 title_text_colour)
        : title_(title), background_(background), title_bar_colour_(title_bar_colour), title_text_colour_(title_text_colour) {
        bounds_ = bounds;
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, background_);
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, TITLE_BAR_HEIGHT, title_bar_colour_);

        float text_width = 0.0f;
        float text_height = 0.0f;
        C2D::GetTextSize(TEXT_SIZE, &text_width, &text_height, title_);
        C2D::Text(bounds_.x + TEXT_MARGIN,
                  bounds_.y + (TITLE_BAR_HEIGHT - text_height) / 2.0f,
                  TEXT_SIZE, title_text_colour_, title_);

        ContainerControl::Draw();
    }

    void SetTitle(const std::string &title) { title_ = title; }
    const std::string &Title() const { return title_; }

private:
    static constexpr float TITLE_BAR_HEIGHT = 24.0f;
    static constexpr float TEXT_SIZE = 0.5f;
    static constexpr float TEXT_MARGIN = 6.0f;

    std::string title_;
    u32 background_;
    u32 title_bar_colour_;
    u32 title_text_colour_;
};

}

#endif
