#include "settings_screen.h"

#include "c2d_helper.h"
#include "cia.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "net.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {

    static const std::string update_error = "Could not connect to network.";
    static const std::string update_success = "Update installed. Please re-run the application";
    static const std::string no_updates = "You are already on the latest version";

SettingsScreen::SettingsScreen(UI::Screen *owner, MenuItem *item)
    : owner_(owner), item_(item) {
    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

/* The theme toggles from inside this screen, so the clear colours
 * must be re-applied every frame. */
void SettingsScreen::OnFrame(u64 delta_time) {
    (void)delta_time;

    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

void SettingsScreen::DrawTop() {
    owner_->DrawTop();
}

void SettingsScreen::DrawBottom() {
    owner_->DrawBottom();

    C2D::Rect(0, 20, 400, 35, cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
    C2D::Rect(0, 55, 320, 185, cfg.dark_theme? BLACK_BG : WHITE);
    if (page_ != GENERAL_SETTINGS)
        C2D::Image(icon_back, 5, 25);

    C2D::Rect(0, 55 + (selection_ * sel_dist), 320, sel_dist, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

    switch(page_) {
        case GENERAL_SETTINGS:
            DisplayGeneral();
            break;

        case SORT_SETTINGS:
            DisplaySort();
            break;

        case UPDATE_SETTINGS:
            DisplayUpdatePage();
            break;
    }

    if (update_popup_ && page_ == UPDATE_SETTINGS)
        DisplayUpdatePopup();
}

void SettingsScreen::DisplayGeneral() {
    C2D::Text(10, 30, 0.44f, WHITE, "Settings");

    C2D::Text(10, 58, 0.44f, cfg.dark_theme? WHITE : BLACK, "Sort by");
    C2D::Text(10, 74, 0.42f, cfg.dark_theme? WHITE : BLACK, "Select between various sorting options.");
    C2D::Text(10, 98, 0.44f, cfg.dark_theme? WHITE : BLACK, "Dark theme");
    C2D::Text(10, 114, 0.42f, cfg.dark_theme? WHITE : BLACK, "Enables dark theme mode.");
    C2D::Text(10, 138, 0.44f, cfg.dark_theme? WHITE : BLACK, "Developer options");
    C2D::Text(10, 154, 0.42f, cfg.dark_theme? WHITE : BLACK, "Enable logging and fs access to NAND.");
    C2D::Text(10, 178, 0.44f, cfg.dark_theme? WHITE : BLACK, "File filter");
    C2D::Textf(170, 178, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "%s", (cfg.filter >= 0)? "Homebrew (.3dsx)" : "All files");
    C2D::Text(10, 206, 0.42f, cfg.dark_theme? WHITE : BLACK, "Show only one type of file in the browser.");
    C2D::Text(10, 230, 0.44f, cfg.dark_theme? WHITE : BLACK, "Check for update");

    if (cfg.dark_theme)
        C2D::Image(cfg.dark_theme? icon_toggle_dark_on : icon_toggle_on, 270, 97);
    else
        C2D::Image(icon_toggle_off, 270, 97);

    C2D::Image(cfg.dev_options? (cfg.dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 270, 137);
}

void SettingsScreen::DisplaySort() {
    C2D::Text(35, 30, 0.44f, WHITE, "Sorting Options");

    C2D::Text(10, 58, 0.44f, cfg.dark_theme? WHITE : BLACK, "Alphabetical");
    C2D::Text(10, 74, 0.42f, cfg.dark_theme? WHITE : BLACK, "Sort alphabetically in ascending order.");
    C2D::Text(10, 98, 0.44f, cfg.dark_theme? WHITE : BLACK, "Alphabetical");
    C2D::Text(10, 114, 0.42f, cfg.dark_theme? WHITE : BLACK, "Sort alphabetically in descending order.");
    C2D::Text(10, 138, 0.44f, cfg.dark_theme? WHITE : BLACK, "Size");
    C2D::Text(10, 154, 0.42f, cfg.dark_theme? WHITE : BLACK, "Sort by size (largest first).");
    C2D::Text(10, 178, 0.44f, cfg.dark_theme? WHITE : BLACK, "Size");
    C2D::Text(10, 194, 0.42f, cfg.dark_theme? WHITE : BLACK, "Sort by size (smallest first).");

    C2D::Image(cfg.sort == 0? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 60);
    C2D::Image(cfg.sort == 1? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 100);
    C2D::Image(cfg.sort == 2? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 140);
    C2D::Image(cfg.sort == 3? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 270, 180);
}

void SettingsScreen::DisplayUpdatePage() {
    C2D::Text(35, 30, 0.44f, WHITE, "Updates");

    C2D::Text(10, 58, 0.44f, cfg.dark_theme? WHITE : BLACK, "Check for updates");
    C2D::Text(10, 74, 0.42f, cfg.dark_theme? WHITE : BLACK, "Downloads and installs the latest version.");
    C2D::Text(10, 98, 0.44f, cfg.dark_theme? WHITE : BLACK, "About");
    C2D::Textf(10, 114, 0.42f, cfg.dark_theme? WHITE : BLACK, "3DShell v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    C2D::Text(10, 138, 0.44f, cfg.dark_theme? WHITE : BLACK, "Author: Joel16");
    C2D::Text(10, 154, 0.42f, cfg.dark_theme? WHITE : BLACK, "Assets: Preetisketch/CyanogenMod/LineageOS");
}

void SettingsScreen::DisplayUpdatePopup() {
    C2D::Image(cfg.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

    bool done = UpdateDone();
    float text_width = 0.0f;

    if (!network_status_) {
        C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Error");
        C2D::GetTextSize(0.42f, &text_width, nullptr, update_error);
        C2D::Text(((320 - text_width) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, update_error);
    }
    else if (network_status_ && update_available_ && (!tag_name_.empty()) && (!done)) {
        C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Update Available");
        C2D::Textf(((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
            "Do you wish to download and install %s?", tag_name_.c_str());
    }
    else if (!update_available_) {
        C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "No Updates");
        C2D::GetTextSize(0.42f, &text_width, nullptr, no_updates);
        C2D::Text(((320 - text_width) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, no_updates);
    }
    else if (done) {
        C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Update Successful");
        C2D::GetTextSize(0.42f, &text_width, nullptr, update_success);
        C2D::Text(((320 - text_width) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, update_success);
    }

    float confirm_width = 0.0f, confirm_height = 0.0f, cancel_width = 0.0f, cancel_height = 0.0f;
    C2D::GetTextSize(0.42f, &confirm_width, &confirm_height, "YES");
    C2D::GetTextSize(0.42f, &cancel_width, &cancel_height, "NO");
    confirm_popup_width_ = confirm_width;
    confirm_popup_height_ = confirm_height;
    cancel_popup_width_ = cancel_width;
    cancel_popup_height_ = cancel_height;

    if (selection_ == 0)
        C2D::Rect((288 - cancel_width) - 5, (159 - cancel_height) - 5, cancel_width + 10, cancel_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (selection_ == 1)
        C2D::Rect((248 - confirm_width) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

    if ((!done) && network_status_ && update_available_)
        C2D::Text(248 - confirm_width, (159 - confirm_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");

    C2D::Text(288 - cancel_width, (159 - cancel_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, ((!network_status_) || done || (!update_available_))? "OK" : "NO");
}

void SettingsScreen::OnKeyHeld(u32 k_held) {
    (void)k_held;
    Touch::Update();
}

void SettingsScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    switch(page_) {
        case GENERAL_SETTINGS:
            ControlGeneral(k_down);
            break;

        case SORT_SETTINGS:
            ControlSort(k_down);
            break;

        case UPDATE_SETTINGS:
            ControlUpdatePage(k_down);
            break;
    }
}

void SettingsScreen::ControlGeneral(u32 k_down) {
    if (k_down & KEY_DUP)
        selection_--;
    else if (k_down & KEY_DDOWN)
        selection_++;

    if (k_down & KEY_A) {
        switch(selection_) {
            case 0:
                page_ = SORT_SETTINGS;
                selection_ = 0;
                break;

            case 1:
                cfg.dark_theme = !cfg.dark_theme;
                Config::Save(cfg);
                break;

            case 2:
                cfg.dev_options = !cfg.dev_options;
                Config::Save(cfg);
                break;

            case 3:
                /* Cycle: show everything <-> only .3dsx payloads. */
                cfg.filter = (cfg.filter >= 0)? -1 : FileType3dsx;
                Config::Save(cfg);
                break;

            case 4:
                page_ = UPDATE_SETTINGS;
                selection_ = 0;
                break;
        }
    }
    else if (k_down & KEY_B) {
        selection_ = 0;
        UI::ScreenManager::Instance().Pop();
    }

    if (Touch::Rect(0, 55, 320, 94)) {
        selection_ = 0;

        if (k_down & KEY_TOUCH) {
            page_ = SORT_SETTINGS;
            selection_ = 0;
        }
    }
    else if (Touch::Rect(0, 95, 320, 134)) {
        selection_ = 1;

        if (k_down & KEY_TOUCH) {
            cfg.dark_theme = !cfg.dark_theme;
            Config::Save(cfg);
        }
    }
    else if (Touch::Rect(0, 135, 320, 174)) {
        selection_ = 2;

        if (k_down & KEY_TOUCH) {
            cfg.dev_options = !cfg.dev_options;
            Config::Save(cfg);
        }
    }
    else if (Touch::Rect(0, 175, 320, 200)) {
        selection_ = 3;

        if (k_down & KEY_TOUCH) {
            cfg.filter = (cfg.filter >= 0)? -1 : FileType3dsx;
            Config::Save(cfg);
        }
    }
    else if (Touch::Rect(0, 205, 320, 239)) {
        selection_ = 4;

        if (k_down & KEY_TOUCH) {
            page_ = UPDATE_SETTINGS;
            selection_ = 0;
        }
    }

    Utils::SetBounds(&selection_, 0, 4);
}

void SettingsScreen::ControlSort(u32 k_down) {
    if (k_down & KEY_DUP)
        selection_--;
    else if (k_down & KEY_DDOWN)
        selection_++;
    else if (k_down & KEY_A) {
        cfg.sort = selection_;
        Config::Save(cfg);
        FS::GetDirList(cfg.cwd, item_->entries);
    }
    else if (k_down & KEY_B) {
        selection_ = 0;
        page_ = GENERAL_SETTINGS;
    }

    if (Touch::Rect(0, 55, 320, 94)) {
        selection_ = 0;

        if (k_down & KEY_TOUCH) {
            cfg.sort = selection_;
            Config::Save(cfg);
            FS::GetDirList(cfg.cwd, item_->entries);
        }
    }
    else if (Touch::Rect(0, 95, 320, 134)) {
        selection_ = 1;

        if (k_down & KEY_TOUCH) {
            cfg.sort = selection_;
            Config::Save(cfg);
            FS::GetDirList(cfg.cwd, item_->entries);
        }
    }
    else if (Touch::Rect(0, 135, 320, 174)) {
        selection_ = 2;

        if (k_down & KEY_TOUCH) {
            cfg.sort = selection_;
            Config::Save(cfg);
            FS::GetDirList(cfg.cwd, item_->entries);
        }
    }
    else if (Touch::Rect(0, 175, 320, 215)) {
        selection_ = 3;

        if (k_down & KEY_TOUCH) {
            cfg.sort = selection_;
            Config::Save(cfg);
            FS::GetDirList(cfg.cwd, item_->entries);
        }
    }
    else if (Touch::Rect(5, 25, 30, 50)) {
        if (k_down & KEY_TOUCH) {
            selection_ = 0;
            page_ = GENERAL_SETTINGS;
        }
    }

    Utils::SetBounds(&selection_, 0, 3);
}

void SettingsScreen::CheckForUpdate() {
    Net::Init();
    network_status_ = Net::GetNetworkStatus();
    tag_name_ = Net::GetLatestReleaseJSON();
    update_available_ = Net::GetAvailableUpdate(tag_name_);
    update_popup_ = true;
    Net::Exit();
}

void SettingsScreen::ControlUpdatePage(u32 k_down) {
    if (update_popup_) {
        bool done = UpdateDone();

        if (k_down & KEY_RIGHT)
            selection_++;
        else if (k_down & KEY_LEFT)
            selection_--;

        if (k_down & KEY_A) {
            if (selection_ == 1)
                GUI::DownloadHelper(tag_name_);
            else if (!done)
                update_popup_ = false;
            else if (done)
                UI::ScreenManager::Instance().Exit();
        }
        else if (k_down & KEY_B)
            update_popup_ = false;

        if (Touch::Rect((288 - cancel_popup_width_) - 5, (159 - cancel_popup_height_) - 5, ((288 - cancel_popup_width_) - 5) + cancel_popup_width_ + 10,
            ((159 - cancel_popup_height_) - 5) + cancel_popup_height_ + 10)) {
            selection_ = 0;

            if (k_down & KEY_TOUCH) {
                if (!done)
                    update_popup_ = false;
                else if (done)
                    UI::ScreenManager::Instance().Exit();
            }
        }
        else if (Touch::Rect((248 - confirm_popup_width_) - 5, (159 - confirm_popup_height_) - 5, ((248 - confirm_popup_width_) - 5) + confirm_popup_width_ + 10,
            ((159 - confirm_popup_height_) - 5) + confirm_popup_height_ + 10)) {
            selection_ = 1;

            if ((k_down & KEY_TOUCH) && (!done) && network_status_ && update_available_)
                GUI::DownloadHelper(tag_name_);
        }

        if (done || (!network_status_) || (!update_available_))
            Utils::SetBounds(&selection_, 0, 0);
        else
            Utils::SetBounds(&selection_, 0, 1);

        return;
    }

    if (k_down & KEY_DUP)
        selection_--;
    else if (k_down & KEY_DDOWN)
        selection_++;
    else if (k_down & KEY_A) {
        if (selection_ == 0)
            CheckForUpdate();
    }
    else if (k_down & KEY_B) {
        selection_ = 0;
        page_ = GENERAL_SETTINGS;
    }

    if (Touch::Rect(0, 55, 320, 94)) {
        selection_ = 0;

        if (k_down & KEY_TOUCH)
            CheckForUpdate();
    }
    else if (Touch::Rect(5, 25, 30, 50)) {
        if (k_down & KEY_TOUCH) {
            selection_ = 0;
            page_ = GENERAL_SETTINGS;
        }
    }

    Utils::SetBounds(&selection_, 0, 2);
}

}
