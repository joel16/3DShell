#include "container_control.h"

namespace UI {

void ContainerControl::Add(std::unique_ptr<Control> control) {
    controls_.push_back(std::move(control));
}

void ContainerControl::Clear() {
    controls_.clear();
    touch_capture_ = nullptr;
}

void ContainerControl::Draw() {
    for (auto &control : controls_) {
        if (control->Visible())
            control->Draw();
    }
}

void ContainerControl::OnKeyDown(u32 k_down) {
    for (auto &control : controls_) {
        if (control->Visible() && control->Enabled())
            control->OnKeyDown(k_down);
    }
}

void ContainerControl::OnKeyHeld(u32 k_held) {
    for (auto &control : controls_) {
        if (control->Visible() && control->Enabled())
            control->OnKeyHeld(k_held);
    }
}

void ContainerControl::OnTouch(const touchPosition &touch, bool down) {
    if (!enabled_ || !visible_) {
        touch_was_down_ = false;
        touch_capture_ = nullptr;
        return;
    }

    if (down) {
        if (!touch_was_down_) {
            touch_capture_ = nullptr;
            for (auto it = controls_.rbegin(); it != controls_.rend(); ++it) {
                Control *control = it->get();
                if (control->Visible() && control->Enabled() && control->Bounds().Contains(touch)) {
                    touch_capture_ = control;
                    break;
                }
            }
        }

        if (touch_capture_)
            touch_capture_->OnTouch(touch, true);
    } else if (touch_was_down_ && touch_capture_) {
        touch_capture_->OnTouch(touch, false);
        touch_capture_ = nullptr;
    }

    touch_was_down_ = down;
}

}
