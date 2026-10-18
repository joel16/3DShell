#include "properties_screen.h"

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "touch.h"

namespace GUI {

PropertiesScreen::PropertiesScreen(UI::Screen *owner, MenuItem *item)
    : owner_(owner), item_(item) {
    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

void PropertiesScreen::OnFrame(u64 delta_time) {
    (void)delta_time;

    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

void PropertiesScreen::DrawTop() {
    owner_->DrawTop();
}

void PropertiesScreen::DrawBottom() {
    owner_->DrawBottom();

    DisplayProperties(item_);

    float ok_width = 0.0f, ok_height = 0.0f;
    C2D::GetTextSize(0.42f, &ok_width, &ok_height, "OK");
    ok_rect_ = UI::Rect{(253.0f - ok_width) - 5.0f, (218.0f - ok_height) - 5.0f, ok_width + 10.0f, ok_height + 10.0f};
}

void PropertiesScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    if (k_down & (KEY_A | KEY_B))
        Close();
}

void PropertiesScreen::OnTouch(const touchPosition &touch, bool down) {
    if (!down) {
        touching_ = false;
        return;
    }

    if (touching_)
        return;

    touching_ = true;

    if (ok_rect_.Contains(touch))
        Close();
}

void PropertiesScreen::Close() {
    UI::ScreenManager::Instance().Pop();
}

}
