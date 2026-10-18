#ifndef _3D_SHELL_SETTINGS_SCREEN_H
#define _3D_SHELL_SETTINGS_SCREEN_H

#include "gui.h"

#include "screen.h"
#include "screen_manager.h"

namespace GUI {

class SettingsScreen : public UI::Screen {
public:
    SettingsScreen(UI::Screen *owner, MenuItem *item);

    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    enum SettingsPage {
        GENERAL_SETTINGS,
        SORT_SETTINGS,
        UPDATE_SETTINGS
    };

    void DisplayGeneral();
    void DisplaySort();
    void DisplayUpdatePage();
    void DisplayUpdatePopup();

    void ControlGeneral(u32 k_down);
    void ControlSort(u32 k_down);
    void ControlUpdatePage(u32 k_down);
    void CheckForUpdate();

    UI::Screen *owner_;
    MenuItem *item_;
    SettingsPage page_ = GENERAL_SETTINGS;
    int selection_ = 0;
    static const int sel_dist = 40;
    std::string tag_name_;
    bool network_status_ = false;
    bool update_available_ = false;
    bool update_popup_ = false;
    bool bar_touching_ = false;
    float confirm_popup_width_ = 0.0f;
    float confirm_popup_height_ = 0.0f;
    float cancel_popup_width_ = 0.0f;
    float cancel_popup_height_ = 0.0f;
};

}

#endif
