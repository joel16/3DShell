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

namespace GUI {

/* GNOME-style app drawer (list mode): shows the installed applications
 * inside the bottom panel. Each entry carries its own launch callback. */
class AppDrawerPanel : public GUI::PanelApp {
public:
    struct AppEntry {
        const char *name;
        C2D_Image icon;
        std::function<void()> launch;
    };

    void AddApp(const char *name, C2D_Image icon, std::function<void()> launch) {
        AppEntry entry;
        entry.name = name;
        entry.icon = icon;
        entry.launch = std::move(launch);
        apps_.push_back(std::move(entry));
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, cfg.dark_theme? BLACK_BG : WHITE);

        C2D::Textf(10, bounds_.y + 10, 0.6f, cfg.dark_theme? WHITE : BLACK, "Apps");

        float y = bounds_.y + 40;
        for (int i = 0; i < static_cast<int>(apps_.size()); i++) {
            const AppEntry &entry = apps_[i];

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

        Utils::SetBounds(&selection_, 0, static_cast<int>(apps_.size()) - 1);

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

        int row = static_cast<int>((touch.py - (bounds_.y + 40)) / ROW_HEIGHT);
        if ((row >= 0) && (row < static_cast<int>(apps_.size()))) {
            selection_ = row;
            Launch();
        }
    }

private:
    void Launch() {
        if (!apps_.empty())
            apps_[selection_].launch();
    }

    std::vector<AppEntry> apps_;
    int selection_ = 0;
    bool touching_ = false;
    static constexpr float ROW_HEIGHT = 40.0f;
};

}

#endif
