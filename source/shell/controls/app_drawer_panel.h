#ifndef _3D_SHELL_APP_DRAWER_PANEL_H
#define _3D_SHELL_APP_DRAWER_PANEL_H

#include <functional>
#include <vector>

#include <panel_app.h>
#include <textures.h>
#include <c2d_helper.h>
#include <colours.h>
#include <config.h>
#include <utils.h>

#include "apps.h"

namespace GUI {

/* GNOME-style app drawer (list mode): lists the applications from the
 * system registry. Fullscreen apps are handed to LaunchScreen; panel
 * apps to SwitchPanel (both wired by the desktop). */
class AppDrawerPanel : public GUI::PanelApp {
public:
    /* Host wiring. */
    std::function<void(std::unique_ptr<UI::Screen>)> LaunchScreen;
    std::function<void(Apps::Id, std::unique_ptr<GUI::PanelApp>)> SwitchPanel;

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, cfg.dark_theme? BLACK_BG : WHITE);

        C2D::Textf(10, bounds_.y + 10, 0.6f, cfg.dark_theme? WHITE : BLACK, "Apps");

        const std::vector<Apps::Entry> &apps = Apps::List();
        float y = bounds_.y + 40;

        for (int i = 0; i < static_cast<int>(apps.size()); i++) {
            const Apps::Entry &entry = apps[i];

            if (selection_ == i)
                C2D::Rect(bounds_.x, y, bounds_.w, ROW_HEIGHT, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

            C2D::Image(entry.icon, 20, y + 10);
            C2D::Text(60, y + 12, 0.5f, cfg.dark_theme? WHITE : BLACK, entry.name);
            y += ROW_HEIGHT;
        }
    }

    void OnKeyDown(u32 k_down) override {
        if (k_down & KEY_DUP)
            selection_--;
        else if (k_down & KEY_DDOWN)
            selection_++;

        const int count = static_cast<int>(Apps::List().size());
        Utils::SetBounds(&selection_, 0, count - 1);

        if (k_down & KEY_A)
            Launch();
        else if (k_down & KEY_B)
            Close();
    }

    void OnTouch(const touchPosition &touch, bool down) override {
        if (!down) {
            touching_ = false;
            return;
        }

        if (touching_)
            return;

        touching_ = true;

        const int count = static_cast<int>(Apps::List().size());
        int row = static_cast<int>((touch.py - (bounds_.y + 40)) / ROW_HEIGHT);
        if ((row >= 0) && (row < count)) {
            selection_ = row;
            Launch();
        }
    }

private:
    void Launch() {
        const std::vector<Apps::Entry> &apps = Apps::List();
        if (apps.empty())
            return;

        const Apps::Entry &entry = apps[selection_];

        if (entry.is_fullscreen()) {
            if (LaunchScreen)
                LaunchScreen(entry.create_screen());
        }
        else if (SwitchPanel) {
            SwitchPanel(entry.id, entry.create_panel());
        }
    }

    int selection_ = 0;
    bool touching_ = false;
    static constexpr float ROW_HEIGHT = 40.0f;
};

}

#endif
