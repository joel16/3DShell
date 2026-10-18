#include "text_reader_screen.h"

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "touch.h"

namespace GUI {

TextReaderScreen::TextReaderScreen(MenuItem *item)
    : item_(item) {
    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);

    touch_bar_.SetSection(TouchBar::SECTION_NONE);
    touch_bar_.Home().Click = [this]() {
        Notepad::Close();
    };
}

void TextReaderScreen::OnFrame(u64 delta_time) {
    (void)delta_time;

    if (!UI::ScreenManager::Instance().IsCurrent(this))
        return;

    /* The editor closed itself -> refresh the listing and go back. */
    if (!Notepad::IsOpen()) {
        FS::GetDirList(cfg.cwd, item_->entries, cfg.filter);
        ResetCheckbox(item_);
        UI::ScreenManager::Instance().Pop();
    }
}

void TextReaderScreen::DrawTop() {
    status_bar_.Draw();
    C2D::Rect(0, 15, 400, 25, cfg.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
    Notepad::DisplayTop();
}

void TextReaderScreen::DrawBottom() {
    touch_bar_.Draw();
    Notepad::DisplayBottom();
}

void TextReaderScreen::OnTouch(const touchPosition &touch, bool down) {
    bool in_bar = down? (touch.py < 20) : bar_touching_;

    if (in_bar) {
        touch_bar_.OnTouch(touch, down);
        bar_touching_ = down;
    }
}

void TextReaderScreen::OnKeyHeld(u32 k_held) {
    k_held_ = k_held;
    Touch::Update();
}

void TextReaderScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    Notepad::Control(k_down, k_held_);
}

}
