#ifndef _3D_SHELL_UI_SCREEN_H
#define _3D_SHELL_UI_SCREEN_H

#include <3ds.h>
#include <citro2d.h>

#include "container_control.h"

namespace UI {

class Screen {
public:
    virtual ~Screen() = default;

    virtual void OnFrame(u64 delta_time) {}

    virtual void DrawTop() { TopPanel().Draw(); }
    virtual void DrawBottom() { BottomPanel().Draw(); }

    virtual void OnKeyDown(u32 k_down) {}
    virtual void OnKeyHeld(u32 k_held) {}

    virtual void OnTouch(const touchPosition &touch, bool down) {
        BottomPanel().OnTouch(touch, down);
    }

    ContainerControl &TopPanel() { return top_panel_; }
    ContainerControl &BottomPanel() { return bottom_panel_; }

    u32 TopBackground() const { return top_background_; }
    void SetTopBackground(u32 colour) { top_background_ = colour; }

    u32 BottomBackground() const { return bottom_background_; }
    void SetBottomBackground(u32 colour) { bottom_background_ = colour; }

private:
    ContainerControl top_panel_;
    ContainerControl bottom_panel_;
    u32 top_background_ = C2D_Color32(0, 0, 0, 255);
    u32 bottom_background_ = C2D_Color32(0, 0, 0, 255);
};

}

#endif
