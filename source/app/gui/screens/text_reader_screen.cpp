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
    C2D::Rect(0, 0, 400, 15, cfg.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
    C2D::Rect(0, 15, 400, 25, cfg.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
    DisplayStatusBar();
    Notepad::DisplayTop();
}

void TextReaderScreen::DrawBottom() {
    C2D::Rect(0, 0, 320, 20, cfg.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
    DisplayTouchButtons(TOUCH_BAR_NONE);
    Notepad::DisplayBottom();
}

void TextReaderScreen::OnTouch(const touchPosition &touch, bool down) {
    if (!down) {
        touching_ = false;
        return;
    }

    if (touching_)
        return;

    touching_ = true;

    /* Home button closes the editor (same as the old global touch bar). */
    if (UI::Rect(0.0f, 0.0f, 22.0f, 20.0f).Contains(touch))
        Notepad::Close();
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
