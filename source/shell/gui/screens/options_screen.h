#ifndef _3D_SHELL_OPTIONS_SCREEN_H
#define _3D_SHELL_OPTIONS_SCREEN_H

#include "gui.h"

#include "screen.h"
#include "screen_manager.h"

namespace GUI {

class OptionsScreen : public UI::Screen {
public:
    OptionsScreen(UI::Screen *owner, MenuItem *item);

    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    void ResetSelector();
    void HandleMultipleCopy(Result (*func)());
    void CreateFolder();
    void CreateFile();
    void Rename();
    void Copy();
    void Move();
    void CancelAll();

    UI::Screen *owner_;
    MenuItem *item_;
    int row_ = 0;
    int column_ = 0;
    bool options_more_ = false;
    bool touching_ = false;
    bool bar_touching_ = false;
    float cancel_width_ = 0.0f;
    float cancel_height_ = 0.0f;
    UI::Rect cancel_rect_;
};

}

#endif
