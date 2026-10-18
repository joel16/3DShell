#ifndef _3D_SHELL_UI_SCROLL_BAR_H
#define _3D_SHELL_UI_SCROLL_BAR_H

#include "../control.h"
#include "../c2d_helper.h"

namespace UI {

class ScrollBar : public Control {
public:
    ScrollBar(const Rect &bounds, u32 track_colour, u32 thumb_colour)
        : track_colour_(track_colour), thumb_colour_(thumb_colour) {
        bounds_ = bounds;
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, track_colour_);

        if (Scrollable()) {
            float top = ThumbTop();
            float height = ThumbHeight();
            C2D::Rect(bounds_.x, bounds_.y + top, bounds_.w, height, thumb_colour_);
        }
    }

    void OnTouch(const touchPosition &touch, bool down) override {
        float px = static_cast<float>(touch.px);
        float py = static_cast<float>(touch.py);

        if (!down) {
            dragging_ = false;
            return;
        }

        if (!Scrollable())
            return;

        float thumb_height = ThumbHeight();
        float thumb_top = bounds_.y + ThumbTop();

        if (!dragging_) {
            if (py >= thumb_top && py < thumb_top + thumb_height) {
                dragging_ = true;
                drag_offset_ = py - thumb_top;
            }
        }

        if (dragging_) {
            float track_height = bounds_.h - thumb_height;
            float ratio = (track_height > 0.0f) ? (py - drag_offset_ - bounds_.y) / track_height : 0.0f;
            SetValue(static_cast<int>(ratio * max_value_ + 0.5f));
        }
    }

    void SetRange(int item_count, int page_size) {
        item_count_ = item_count > 0 ? item_count : 0;
        page_size_ = page_size > 0 ? page_size : 1;
        max_value_ = item_count_ > page_size_ ? item_count_ - page_size_ : 0;
        Clamp();
    }

    void SetValue(int value) {
        value_ = value;
        Clamp();
    }

    int Value() const { return value_; }

    void ScrollBy(int delta) { SetValue(value_ + delta); }

    bool Scrollable() const { return max_value_ > 0; }

private:
    void Clamp() {
        if (value_ < 0)
            value_ = 0;
        if (value_ > max_value_)
            value_ = max_value_;
    }

    float ThumbHeight() const {
        if (!Scrollable())
            return bounds_.h;
        float ratio = static_cast<float>(page_size_) / static_cast<float>(item_count_);
        float height = ratio * bounds_.h;
        return height < MIN_THUMB_HEIGHT ? MIN_THUMB_HEIGHT : height;
    }

    float ThumbTop() const {
        if (!Scrollable())
            return 0.0f;
        float track_height = bounds_.h - ThumbHeight();
        float ratio = static_cast<float>(value_) / static_cast<float>(max_value_);
        return ratio * track_height;
    }

    static constexpr float MIN_THUMB_HEIGHT = 24.0f;

    int value_ = 0;
    int max_value_ = 0;
    int item_count_ = 0;
    int page_size_ = 1;
    u32 track_colour_;
    u32 thumb_colour_;
    bool dragging_ = false;
    float drag_offset_ = 0.0f;
};

}

#endif
