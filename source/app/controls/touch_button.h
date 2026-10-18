#ifndef _3D_SHELL_TOUCH_BUTTON_H
#define _3D_SHELL_TOUCH_BUTTON_H

#include <algorithm>
#include <functional>

#include <config.h>

#include <control.h>
#include <c2d_helper.h>
#include <colours.h>

namespace GUI {

/* Icon-based touch button: three variants (light, dark, active). The
 * container sets Bounds via layout; the icon is centered inside. Fires
 * Click on release inside bounds. */
class TouchButton : public UI::Control {
public:
    TouchButton(C2D_Image normal, C2D_Image dark, C2D_Image overlay)
        : normal_(normal), dark_(dark), overlay_(overlay) {}

    void Draw() override {
        C2D_Image img = cfg.dark_theme? dark_ : normal_;
        if (active_ || pressed_)
            img = overlay_;

        float x = bounds_.x + ((bounds_.w - static_cast<float>(img.subtex->width)) / 2.0f);
        float y = bounds_.y + ((bounds_.h - static_cast<float>(img.subtex->height)) / 2.0f);
        C2D::Image(img, x, y);
    }

    void OnTouch(const touchPosition &touch, bool down) override {
        bool hit = Bounds().Contains(touch);

        if (down)
            pressed_ = hit;
        else if (pressed_) {
            pressed_ = false;
            if (hit && Click)
                Click();
        }
    }

    void SetActive(bool active) { active_ = active; }

    float IntrinsicWidth() const {
        return std::max({static_cast<float>(normal_.subtex->width),
                         static_cast<float>(dark_.subtex->width),
                         static_cast<float>(overlay_.subtex->width)});
    }

    std::function<void()> Click;

private:
    C2D_Image normal_;
    C2D_Image dark_;
    C2D_Image overlay_;
    bool active_ = false;
    bool pressed_ = false;
};

}

#endif
