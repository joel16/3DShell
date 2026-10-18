#include "delete_screen.h"

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "textures.h"
#include "utils.h"

namespace GUI {

DeleteScreen::DeleteScreen(UI::Screen *owner, MenuItem *item)
    : owner_(owner), item_(item) {
    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

void DeleteScreen::OnFrame(u64 delta_time) {
    (void)delta_time;

    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
}

void DeleteScreen::DrawTop() {
    owner_->DrawTop();
}

void DeleteScreen::DrawBottom() {
    owner_->DrawBottom();

    C2D::Image(cfg.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));
    C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Delete");

    const std::string prompt = "Do you wish to continue?";
    float prompt_width = 0.0f;
    C2D::GetTextSize(0.42f, &prompt_width, nullptr, prompt);
    C2D::Text(((320 - prompt_width) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, prompt);

    float confirm_width = 0.0f, confirm_height = 0.0f, cancel_width = 0.0f, cancel_height = 0.0f;
    C2D::GetTextSize(0.42f, &confirm_width, &confirm_height, "YES");
    C2D::GetTextSize(0.42f, &cancel_width, &cancel_height, "NO");

    if (selection_ == 0)
        C2D::Rect((288 - cancel_width) - 5, (159 - cancel_height) - 5, cancel_width + 10, cancel_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else
        C2D::Rect((248 - confirm_width) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

    C2D::Text(248 - confirm_width, (159 - confirm_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
    C2D::Text(288 - cancel_width, (159 - cancel_height) - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");

    confirm_rect_ = UI::Rect{(248.0f - confirm_width) - 5.0f, (159.0f - confirm_height) - 5.0f, confirm_width + 10.0f, confirm_height + 10.0f};
    cancel_rect_ = UI::Rect{(288.0f - cancel_width) - 5.0f, (159.0f - cancel_height) - 5.0f, cancel_width + 10.0f, cancel_height + 10.0f};
}

void DeleteScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    if (k_down & KEY_RIGHT)
        selection_++;
    else if (k_down & KEY_LEFT)
        selection_--;

    if (k_down & KEY_A) {
        if (selection_ == 1)
            Confirm();
        else
            Cancel();
    }
    else if (k_down & KEY_B)
        Cancel();

    Utils::SetBounds(&selection_, 0, 1);
}

void DeleteScreen::OnTouch(const touchPosition &touch, bool down) {
    /* Touch bar zones forward to the browser (works with a dialog open),
     * including the release so buttons complete their click. */
    bool in_bar = down? (touch.py < 20) : bar_touching_;

    if (in_bar) {
        owner_->OnTouch(touch, down);
        bar_touching_ = down;
        return;
    }

    bar_touching_ = false;

    if (!down) {
        touching_ = false;
        return;
    }

    if (touching_)
        return;

    touching_ = true;

    if (cancel_rect_.Contains(touch)) {
        selection_ = 0;
        Cancel();
    }
    else if (confirm_rect_.Contains(touch)) {
        selection_ = 1;
        Confirm();
    }
}

void DeleteScreen::Confirm() {
    Options::Delete(item_, &selection_);
    UI::ScreenManager::Instance().Pop();
}

void DeleteScreen::Cancel() {
    selection_ = 0;
    UI::ScreenManager::Instance().Pop();
}

}
