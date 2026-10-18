#ifndef _3D_SHELL_TOUCH_BAR_H
#define _3D_SHELL_TOUCH_BAR_H

#include <3ds.h>

#include <config.h>
#include <fs.h>
#include <textures.h>

#include <container_control.h>
#include <layout.h>

#include "touch_button.h"
#include <c2d_helper.h>
#include <colours.h>

namespace GUI {

/* Bottom screen chrome: background bar + six icon buttons laid out in
 * two groups. LEFT group flows from the left edge (home, options,
 * settings); RIGHT group is anchored to the right edge (sd, secure,
 * search). Hiding a button re-packs the group. Each button fires its
 * own event on release; overlay highlights derive from the current
 * section and the active archive. */
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

        auto home = std::make_unique<TouchButton>(icon_home, icon_home_dark, icon_home_overlay);
        auto options = std::make_unique<TouchButton>(icon_options, icon_options_dark, icon_options_overlay);
        auto settings = std::make_unique<TouchButton>(icon_settings, icon_settings_dark, icon_settings_overlay);
        auto sd = std::make_unique<TouchButton>(icon_sd, icon_sd_dark, icon_sd_overlay);
        auto secure = std::make_unique<TouchButton>(icon_secure, icon_secure_dark, icon_secure_overlay);
        auto search = std::make_unique<TouchButton>(icon_search, icon_search, icon_search);

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

        left_items_ = {
            { home_, home_->IntrinsicWidth() },
            { options_, options_->IntrinsicWidth() },
            { settings_, settings_->IntrinsicWidth() }
        };

        right_items_ = {
            { sd_, sd_->IntrinsicWidth() },
            { secure_, secure_->IntrinsicWidth() },
            { search_, search_->IntrinsicWidth() }
        };
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

        Reflow();

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
    void Reflow() {
        const float y = bounds_.y;
        const float height = bounds_.h;
        const float margin = 2.0f;
        const float gap = 4.0f;

        UI::FlowLayout::Flow(left_items_, y, height, bounds_.x + margin, gap, false);
        UI::FlowLayout::Flow(right_items_, y, height, bounds_.x + bounds_.w - margin, gap, true);
    }

    Section section_ = SECTION_NONE;
    TouchButton *home_ = nullptr;
    TouchButton *options_ = nullptr;
    TouchButton *settings_ = nullptr;
    TouchButton *sd_ = nullptr;
    TouchButton *secure_ = nullptr;
    TouchButton *search_ = nullptr;
    std::vector<UI::FlowLayout::Item> left_items_;
    std::vector<UI::FlowLayout::Item> right_items_;
};

}

#endif
