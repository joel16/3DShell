#ifndef _3D_SHELL_TOUCH_BUTTON_H
#define _3D_SHELL_TOUCH_BUTTON_H

#include <functional>

#include <config.h>

#include <control.h>
#include <c2d_helper.h>

namespace GUI {

/* Icon-based touch button: three variants (light, dark, active) and a
 * zone rectangle for hit testing. Fires Click on release inside bounds. */
class TouchButton : public UI::Control {
public:
    TouchButton(C2D_Image normal, C2D_Image dark, C2D_Image overlay, const UI::Rect &zone, float icon_x)
        : normal_(normal), dark_(dark), overlay_(overlay), icon_x_(icon_x) {
        bounds_ = zone;
    }

    void Draw() override {
        C2D_Image img = cfg.dark_theme? dark_ : normal_;
        if (active_ || pressed_)
            img = overlay_;
        C2D::Image(img, icon_x_, bounds_.y);
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

    std::function<void()> Click;

private:
    C2D_Image normal_;
    C2D_Image dark_;
    C2D_Image overlay_;
    float icon_x_;
    bool active_ = false;
    bool pressed_ = false;
};

}

#endif
