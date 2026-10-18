#include "options_screen.h"

#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "delete_screen.h"
#include "fs.h"
#include "osk.h"
#include "properties_screen.h"
#include "textures.h"
#include "utils.h"

/* Clipboard state: survives dialog closes (copy/move two-step flow). */
static bool copy = false, move = false;

namespace GUI {

OptionsScreen::OptionsScreen(UI::Screen *owner, MenuItem *item)
    : owner_(owner), item_(item) {
    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

void OptionsScreen::OnFrame(u64 delta_time) {
    (void)delta_time;

    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

void OptionsScreen::DrawTop() {
    owner_->DrawTop();
}

void OptionsScreen::DrawBottom() {
    owner_->DrawBottom();

    C2D::Image(cfg.dark_theme? options_dialog_dark : options_dialog, 54, 30);
    C2D::Text(61, 34, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Actions");

    C2D::GetTextSize(0.42f, &cancel_width_, &cancel_height_, "CANCEL");

    if (row_ == 0 && column_ == 0)
        C2D::Rect(56, 69, 103, 36, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row_ == 1 && column_ == 0)
        C2D::Rect(160, 69, 103, 36, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row_ == 0 && column_ == 1)
        C2D::Rect(56, 105, 103, 36, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row_ == 1 && column_ == 1)
        C2D::Rect(160, 105, 103, 36, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row_ == 0 && column_ == 2 && !options_more_)
        C2D::Rect(56, 142, 103, 36, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row_ == 1 && column_ == 2 && !options_more_)
        C2D::Rect(160, 142, 103, 36, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (column_ == 3 && !options_more_)
        C2D::Rect((256 - cancel_width_) - 5, (221 - cancel_height_) - 5, cancel_width_ + 10, cancel_height_ + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (column_ == 2 && options_more_)
        C2D::Rect((256 - cancel_width_) - 5, (221 - cancel_height_) - 5, cancel_width_ + 10, cancel_height_ + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

    C2D::Text(256 - cancel_width_, 221 - cancel_height_ - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "CANCEL");

    if (!options_more_) {
        C2D::Text(66, 78, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Properties");
        C2D::Text(66, 114, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, copy? "Paste" : "Copy");
        C2D::Text(66, 150, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Delete");
        C2D::Text(170, 78, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Refresh");
        C2D::Text(170, 114, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, move? "Paste" : "Move");
        C2D::Text(170, 150, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "More...");
    }
    else {
        C2D::Text(66, 78, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "New folder");
        C2D::Text(66, 114, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Rename");
        C2D::Text(170, 78, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "New file");
    }

    cancel_rect_ = UI::Rect{(256.0f - cancel_width_) - 5.0f, (221.0f - cancel_height_) - 5.0f, cancel_width_ + 10.0f, cancel_height_ + 10.0f};
}

void OptionsScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    if (k_down & KEY_RIGHT)
        row_++;
    else if (k_down & KEY_LEFT)
        row_--;

    if (k_down & KEY_DDOWN)
        column_++;
    else if (k_down & KEY_DUP)
        column_--;

    if (!options_more_) {
        Utils::SetBounds(&row_, 0, 1);
        Utils::SetBounds(&column_, 0, 3);
    }
    else {
        Utils::SetBounds(&column_, 0, 2);

        if (column_ == 1)
            Utils::SetBounds(&row_, 0, 0);
        else
            Utils::SetBounds(&row_, 0, 1);
    }

    if (k_down & KEY_A) {
        const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item_->entries[item_->selected].name);
        const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());

        if (row_ == 0) {
            if (!options_more_) {
                if (column_ == 0)
                    UI::ScreenManager::Instance().Push(std::make_unique<PropertiesScreen>(owner_, item_));
                else if (column_ == 1)
                    Copy();
                else if (column_ == 2)
                    UI::ScreenManager::Instance().Push(std::make_unique<DeleteScreen>(owner_, item_));
            }
            else {
                if (column_ == 0)
                    CreateFolder();
                else if (column_ == 1)
                    Rename();
            }
        }
        else if (row_ == 1) {
            if (!options_more_) {
                if (column_ == 0) {
                    FS::GetDirList(cfg.cwd, item_->entries);
                    ResetSelector();
                    options_more_ = false;
                    item_->selected = 0;
                    UI::ScreenManager::Instance().Pop();
                }
                else if (column_ == 1)
                    Move();
                else if (column_ == 2) {
                    ResetSelector();
                    options_more_ = true;
                }
            }
            else {
                if (column_ == 0)
                    CreateFile();
            }
        }
        if (column_ == 3)
            CancelAll();
    }
    if (k_down & KEY_B) {
        ResetSelector();

        if (!options_more_)
            UI::ScreenManager::Instance().Pop();
        else
            options_more_ = false;
    }
}

void OptionsScreen::OnTouch(const touchPosition &touch, bool down) {
    if (!down) {
        touching_ = false;
        return;
    }

    if (touching_)
        return;

    touching_ = true;

    const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item_->entries[item_->selected].name);
    const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());

    if (UI::Rect(56, 69, 103, 36).Contains(touch)) {
        row_ = 0;
        column_ = 0;

        if (!options_more_)
            UI::ScreenManager::Instance().Push(std::make_unique<PropertiesScreen>(owner_, item_));
        else
            CreateFolder();
    }
    else if (UI::Rect(160, 69, 103, 36).Contains(touch)) {
        row_ = 1;
        column_ = 0;

        if (!options_more_) {
            FS::GetDirList(cfg.cwd, item_->entries);
            ResetSelector();
            options_more_ = false;
            item_->selected = 0;
            UI::ScreenManager::Instance().Pop();
        }
        else
            CreateFile();
    }
    else if (UI::Rect(56, 105, 103, 36).Contains(touch)) {
        row_ = 0;
        column_ = 1;

        if (!options_more_)
            Copy();
        else
            Rename();
    }
    else if (UI::Rect(160, 105, 103, 36).Contains(touch) && !options_more_) {
        row_ = 1;
        column_ = 1;
        Move();
    }
    else if (UI::Rect(56, 142, 103, 36).Contains(touch) && !options_more_) {
        row_ = 0;
        column_ = 2;
        UI::ScreenManager::Instance().Push(std::make_unique<DeleteScreen>(owner_, item_));
    }
    else if (UI::Rect(160, 142, 103, 36).Contains(touch) && !options_more_) {
        row_ = 1;
        column_ = 2;
        ResetSelector();
        options_more_ = true;
    }
    else if (cancel_rect_.Contains(touch)) {
        CancelAll();
    }
}

void OptionsScreen::ResetSelector() {
    row_ = 0;
    column_ = 0;
}

void OptionsScreen::HandleMultipleCopy(Result (*func)()) {
    Result ret = 0;
    std::vector<FS_DirectoryEntry> entries;

    if (R_FAILED(ret = FS::GetDirList(item_->checked_cwd.data(), entries)))
        return;

    for (u32 i = 0; i < item_->checked_copy.size(); i++) {
        if (item_->checked_copy.at(i)) {
            FS::Copy(&entries[i], item_->checked_cwd);
            if (R_FAILED((*func)())) {
                FS::GetDirList(cfg.cwd, item_->entries);
                ResetCheckbox(item_);
                break;
            }
        }
    }

    FS::GetDirList(cfg.cwd, item_->entries);
    ResetCheckbox(item_);
    entries.clear();
}

void OptionsScreen::CreateFolder() {
    std::string path = cfg.cwd;
    std::string name = OSK::GetText("New Folder", "Enter folder name");
    path.append(name);
    std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());

    if (R_SUCCEEDED(FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0))) {
        FS::GetDirList(cfg.cwd, item_->entries);
        ResetCheckbox(item_);
    }
}

void OptionsScreen::CreateFile() {
    std::string path = cfg.cwd;
    std::string name = OSK::GetText("New File", "Enter file name");
    path.append(name);
    std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());

    if (R_SUCCEEDED(FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0, 0))) {
        FS::GetDirList(cfg.cwd, item_->entries);
        ResetCheckbox(item_);
    }
}

void OptionsScreen::Rename() {
    const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item_->entries[item_->selected].name);
    const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());

    std::string path = OSK::GetText(filename, "Enter new name");

    if (R_SUCCEEDED(FS::Rename(&item_->entries[item_->selected], path.c_str()))) {
        FS::GetDirList(cfg.cwd, item_->entries);
        ResetSelector();
        options_more_ = false;
        UI::ScreenManager::Instance().Pop();
    }
}

void OptionsScreen::Copy() {
    if (!copy) {
        if ((item_->checked_count >= 1) && (item_->checked_cwd.compare(cfg.cwd) != 0))
            ResetCheckbox(item_);
        if (item_->checked_count <= 1)
            FS::Copy(&item_->entries[item_->selected], cfg.cwd);

        copy = !copy;
        UI::ScreenManager::Instance().Pop();
    }
    else {
        if ((item_->checked_count > 1) && (item_->checked_cwd.compare(cfg.cwd) != 0))
            HandleMultipleCopy(&FS::Paste);
        else {
            if (R_SUCCEEDED(FS::Paste())) {
                FS::GetDirList(cfg.cwd, item_->entries);
                ResetCheckbox(item_);
            }
        }

        RecalcStorageSize(item_);
        copy = !copy;
        UI::ScreenManager::Instance().Pop();
    }
}

void OptionsScreen::Move() {
    if (!move) {
        if ((item_->checked_count >= 1) && (item_->checked_cwd.compare(cfg.cwd) != 0))
            ResetCheckbox(item_);

        if (item_->checked_count <= 1)
            FS::Copy(&item_->entries[item_->selected], cfg.cwd);
    }
    else {
        if ((item_->checked_count > 1) && (item_->checked_cwd.compare(cfg.cwd) != 0))
            HandleMultipleCopy(&FS::Move);
        else if (R_SUCCEEDED(FS::Move())) {
            FS::GetDirList(cfg.cwd, item_->entries);
            ResetCheckbox(item_);
        }
    }

    move = !move;
    UI::ScreenManager::Instance().Pop();
}

void OptionsScreen::CancelAll() {
    copy = false;
    move = false;
    ResetSelector();
    options_more_ = false;
    UI::ScreenManager::Instance().Pop();
}

}
