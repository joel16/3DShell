#ifndef _3D_SHELL_SETTINGS_PANEL_H
#define _3D_SHELL_SETTINGS_PANEL_H

#include <functional>

#include "panel_app.h"

namespace GUI {

class SettingsPanel : public GUI::PanelApp {
public:
    /* Fired when a setting alters the directory listing (sort changes). */
    std::function<void()> ListingChanged;

    void Draw() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;
    void OnFrame(u64 delta_time) override;

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

    SettingsPage page_ = GENERAL_SETTINGS;
    int selection_ = 0;
    static const int sel_dist = 40;
    std::string tag_name_;
    bool network_status_ = false;
    bool update_available_ = false;
    bool update_popup_ = false;
    float confirm_popup_width_ = 0.0f;
    float confirm_popup_height_ = 0.0f;
    float cancel_popup_width_ = 0.0f;
    float cancel_popup_height_ = 0.0f;
};

}

#endif
