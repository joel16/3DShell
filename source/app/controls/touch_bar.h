#ifndef _3D_SHELL_TOUCH_BAR_H
#define _3D_SHELL_TOUCH_BAR_H

#include <3ds.h>

#include <config.h>
#include <fs.h>
#include <textures.h>

#include <container_control.h>
#include <touch_button.h>
#include <c2d_helper.h>
#include <colours.h>

namespace GUI {

/* Bottom screen chrome: background bar + six icon buttons. Owns the
 * hit zones; each button fires its own event on release. Overlay
 * highlights derive from the current section and the active archive. */
class TouchBar : public UI::ContainerControl {
public:
    enum Section {
        SECTION_NONE,
        SECTION_BROWSER,
        SECTION_ACTIONS,
        SECTION_SETTINGS
    };

    TouchBar() {
        bounds_ = UI::Rect{0.0f, 0.0f, 320.0f, 20.0f};

        auto home = std::make_unique<TouchButton>(icon_home, icon_home_dark, icon_home_overlay, UI::Rect{0, 0, 23, 20}, 2.0f);
        auto options = std::make_unique<TouchButton>(icon_options, icon_options_dark, icon_options_overlay, UI::Rect{23, 0, 47, 20}, 25.0f);
        auto settings = std::make_unique<TouchButton>(icon_settings, icon_settings_dark, icon_settings_overlay, UI::Rect{48, 0, 72, 20}, 50.0f);
        auto sd = std::make_unique<TouchButton>(icon_sd, icon_sd_dark, icon_sd_overlay, UI::Rect{247, 0, 272, 20}, 250.0f);
        auto secure = std::make_unique<TouchButton>(icon_secure, icon_secure_dark, icon_secure_overlay, UI::Rect{273, 0, 292, 20}, 275.0f);
        auto search = std::make_unique<TouchButton>(icon_search, icon_search, icon_search, UI::Rect{293, 0, 320, 20}, 300.0f);

        home_ = home.get();
        options_ = options.get();
        settings_ = settings.get();
        sd_ = sd.get();
        secure_ = secure.get();
        search_ = search.get();

        Add(std::move(home));
        Add(std::move(options));
        Add(std::move(settings));
        Add(std::move(sd));
        Add(std::move(secure));
        Add(std::move(search));
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, cfg.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);

        home_->SetActive(section_ == SECTION_BROWSER);
        options_->SetActive(section_ == SECTION_ACTIONS);
        settings_->SetActive(section_ == SECTION_SETTINGS);
        sd_->SetActive(archive == sdmc_archive);
        /* NAND access is a developer-only feature: hide the button
         * entirely instead of showing one that does nothing. */
        secure_->SetVisible(cfg.dev_options);
        secure_->SetActive(archive == nand_archive);

        ContainerControl::Draw();
    }

    void SetSection(Section section) { section_ = section; }

    TouchButton &Home() { return *home_; }
    TouchButton &Options() { return *options_; }
    TouchButton &Settings() { return *settings_; }
    TouchButton &SD() { return *sd_; }
    TouchButton &Secure() { return *secure_; }
    TouchButton &Search() { return *search_; }

private:
    Section section_ = SECTION_NONE;
    TouchButton *home_ = nullptr;
    TouchButton *options_ = nullptr;
    TouchButton *settings_ = nullptr;
    TouchButton *sd_ = nullptr;
    TouchButton *secure_ = nullptr;
    TouchButton *search_ = nullptr;
};

}

#endif
