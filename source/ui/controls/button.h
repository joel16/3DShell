#ifndef _3D_SHELL_UI_BUTTON_H
#define _3D_SHELL_UI_BUTTON_H

#include <functional>
#include <string>

#include "../control.h"
#include "../c2d_helper.h"

namespace UI {

class Button : public Control {
public:
    Button(const std::string &text, const Rect &bounds, u32 background, u32 text_colour)
        : text_(text), background_(background), text_colour_(text_colour) {
        bounds_ = bounds;
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, background_);

        if (pressed_)
            C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, PRESS_OVERLAY);

        float text_width = 0.0f;
        float text_height = 0.0f;
        C2D::GetTextSize(TEXT_SIZE, &text_width, &text_height, text_);
        C2D::Text(bounds_.x + (bounds_.w - text_width) / 2.0f,
                  bounds_.y + (bounds_.h - text_height) / 2.0f,
                  TEXT_SIZE, text_colour_, text_);
    }

    void OnTouch(const touchPosition &touch, bool down) override {
        bool hit = Bounds().Contains(touch);

        if (down) {
            pressed_ = hit;
        } else if (pressed_) {
            pressed_ = false;
            if (hit && Click)
                Click();
        }
    }

    void SetText(const std::string &text) { text_ = text; }
    const std::string &Text() const { return text_; }

    std::function<void()> Click;

private:
    static constexpr float TEXT_SIZE = 0.5f;
    static constexpr u32 PRESS_OVERLAY = C2D_Color32(0, 0, 0, 60);

    std::string text_;
    u32 background_;
    u32 text_colour_;
    bool pressed_ = false;
};

}

#endif
