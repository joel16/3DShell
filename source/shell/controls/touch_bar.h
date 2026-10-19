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

/* Bottom screen chrome: background bar + icon buttons laid out in two
 * groups. Screens add the buttons they need with AddButton(): the LEFT
 * group flows from the left edge, the RIGHT group is anchored to the
 * right edge. Invisible buttons are skipped by the layout, so the group
 * re-packs. Each button fires its own Click event on release. */
class TouchBar : public UI::ContainerControl {
public:
    enum TouchBarIcon {
        ICON_HOME,
        ICON_FILES,
        ICON_NETLOADER,
        ICON_OPTIONS,
        ICON_SETTINGS,
        ICON_SD,
        ICON_NAND,
        ICON_SEARCH
    };

    enum Section {
        SECTION_NONE,
        SECTION_BROWSER,
        SECTION_ACTIONS,
        SECTION_SETTINGS
    };

    TouchBar() {
        bounds_ = UI::Rect{0.0f, 0.0f, 320.0f, 20.0f};
    }

    TouchButton &AddButton(TouchBarIcon icon, bool right_group) {
        C2D_Image normal, dark, overlay;

        switch (icon) {
            case ICON_HOME:
                normal = icon_home; dark = icon_home_dark; overlay = icon_home_overlay;
                break;
            case ICON_FILES:
                normal = icon_dir; dark = icon_dir_dark; overlay = icon_dir;
                break;
            case ICON_NETLOADER:
                normal = icon_ftp; dark = icon_ftp_dark; overlay = icon_ftp;
                break;
            case ICON_OPTIONS:
                normal = icon_options; dark = icon_options_dark; overlay = icon_options_overlay;
                break;
            case ICON_SETTINGS:
                normal = icon_settings; dark = icon_settings_dark; overlay = icon_settings_overlay;
                break;
            case ICON_SD:
                normal = icon_sd; dark = icon_sd_dark; overlay = icon_sd_overlay;
                break;
            case ICON_NAND:
                normal = icon_secure; dark = icon_secure_dark; overlay = icon_secure_overlay;
                break;
            case ICON_SEARCH:
            default:
                normal = icon_search; dark = icon_search; overlay = icon_search;
                break;
        }

        auto button = std::make_unique<TouchButton>(normal, dark, overlay);
        TouchButton &ref = *button;

        UI::FlowLayout::Item item{&ref, ref.IntrinsicWidth()};
        if (right_group)
            right_items_.push_back(item);
        else
            left_items_.push_back(item);

        Add(std::move(button));
        return ref;
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, cfg.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);

        if (home_)
            home_->SetActive(section_ == SECTION_BROWSER);
        if (options_)
            options_->SetActive(section_ == SECTION_ACTIONS);
        if (settings_)
            settings_->SetActive(section_ == SECTION_SETTINGS);
        if (sd_)
            sd_->SetActive(archive == sdmc_archive);
        if (secure_) {
            /* NAND access is a developer-only feature: hide the button
             * entirely instead of showing one that does nothing. */
            secure_->SetVisible(cfg.dev_options);
            secure_->SetActive(archive == nand_archive);
        }

        Reflow();

        ContainerControl::Draw();
    }

    void SetSection(Section section) { section_ = section; }

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
    std::vector<UI::FlowLayout::Item> left_items_;
    std::vector<UI::FlowLayout::Item> right_items_;
};

}

#endif
