#ifndef _3D_SHELL_UI_CONTROL_H
#define _3D_SHELL_UI_CONTROL_H

#include <3ds.h>

namespace UI {

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    bool Contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }

    bool Contains(const touchPosition &touch) const {
        return Contains(static_cast<float>(touch.px), static_cast<float>(touch.py));
    }
};

class Control {
public:
    virtual ~Control() = default;

    virtual void Draw() {}
    virtual void OnKeyDown(u32 k_down) {}
    virtual void OnKeyHeld(u32 k_held) {}
    virtual void OnTouch(const touchPosition &touch, bool down) {}

    const Rect &Bounds() const { return bounds_; }
    void SetBounds(const Rect &bounds) { bounds_ = bounds; }

    bool Visible() const { return visible_; }
    void SetVisible(bool visible) { visible_ = visible; }

    bool Enabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

protected:
    Rect bounds_;
    bool visible_ = true;
    bool enabled_ = true;
};

}

#endif
