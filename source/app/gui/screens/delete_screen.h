#ifndef _3D_SHELL_DELETE_SCREEN_H
#define _3D_SHELL_DELETE_SCREEN_H

#include "gui.h"

#include "screen.h"
#include "screen_manager.h"

namespace GUI {

class DeleteScreen : public UI::Screen {
public:
    DeleteScreen(UI::Screen *owner, MenuItem *item);

    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    void Confirm();
    void Cancel();

    UI::Screen *owner_;
    MenuItem *item_;
    int selection_ = 0;
    UI::Rect confirm_rect_;
    UI::Rect cancel_rect_;
    bool touching_ = false;
};

}

#endif
