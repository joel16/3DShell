#include "net_loader_panel.h"

#include <c2d_helper.h>
#include <colours.h>
#include <config.h>
#include <screen_manager.h>

namespace GUI {

void NetLoaderPanel::OnFrame(u64 delta_time) {
    (void)delta_time;

    if (!started_) {
        started_ = true;
        NetLoader::Start();
        return;
    }

    /* The worker finished (done, failed or cancelled) -> back to desktop. */
    if (!NetLoader::Busy())
        Close();
}

void NetLoaderPanel::DrawProgressBar(float y, s64 offset, s64 size) {
    s64 total = (size < 1)? 1 : size;
    total = (total < offset)? offset : total;

    C2D::Rect(10, y, 300, 8, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

    float ratio = (static_cast<double>(offset) / static_cast<double>(total));
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    C2D::Rect(10, y, static_cast<float>(ratio * 300.0), 8, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
}

void NetLoaderPanel::Draw() {
    C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, cfg.dark_theme? BLACK_BG : WHITE);

    C2D::Text(10, bounds_.y + 20, 0.6f, cfg.dark_theme? WHITE : BLACK, "NetLoader");
    C2D::Text(10, bounds_.y + 60, 0.45f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, NetLoader::Message());

    if (NetLoader::Busy())
        DrawProgressBar(bounds_.y + 100, NetLoader::Total(), NetLoader::Size());

    /* BACK button (bottom-right corner of the panel). */
    float back_width = 0.0f, back_height = 0.0f;
    C2D::GetTextSize(0.42f, &back_width, &back_height, "BACK");
    back_rect_ = UI::Rect{(bounds_.x + bounds_.w) - back_width - 15.0f,
                          (bounds_.y + bounds_.h) - back_height - 15.0f,
                          back_width + 10.0f, back_height + 10.0f};

    C2D::Rect(back_rect_.x, back_rect_.y, back_rect_.w, back_rect_.h, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    C2D::Text(back_rect_.x + 5, back_rect_.y + 5, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "BACK");

    C2D::Text(10, bounds_.y + 180, 0.40f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "B: back");
}

void NetLoaderPanel::OnTouch(const touchPosition &touch, bool down) {
    if (!down) {
        touching_ = false;
        return;
    }

    if (touching_)
        return;

    touching_ = true;

    if (back_rect_.Contains(touch))
        NetLoader::Cancel();
}

void NetLoaderPanel::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    /* B backs out: cancels the wait; an in-flight transfer aborts at the
     * next chunk boundary. */
    if (k_down & KEY_B)
        NetLoader::Cancel();
}

}
