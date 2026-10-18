#include "screen_manager.h"

#include "c2d_helper.h"

namespace UI {

ScreenManager &ScreenManager::Instance() {
    static ScreenManager instance;
    return instance;
}

void ScreenManager::Run(std::unique_ptr<Screen> root) {
    stack_.clear();
    graveyard_.clear();
    stack_.push_back(std::move(root));
    running_ = true;

    u64 last_time = osGetTime();

    while (running_ && aptMainLoop() && !stack_.empty()) {
        u64 current_time = osGetTime();
        delta_time_ = current_time - last_time;
        last_time = current_time;

        Screen *screen = Current();
        if (!screen)
            break;

        screen->OnFrame(delta_time_);
        screen = Current();
        if (!screen)
            break;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top_screen, screen->TopBackground());
        C2D_TargetClear(bottom_screen, screen->BottomBackground());
        C2D_SceneBegin(top_screen);
        screen->DrawTop();
        C2D_SceneBegin(bottom_screen);
        screen->DrawBottom();
        C2D::Render();

        hidScanInput();

        u32 k_down = hidKeysDown();
        u32 k_held = hidKeysHeld();

        if (Current() == screen)
            screen->OnKeyHeld(k_held);

        if (Current() == screen)
            screen->OnKeyDown(k_down);

        touchPosition touch = {};
        bool touching = (k_held & KEY_TOUCH) != 0;
        if (touching)
            hidTouchRead(&touch);

        if (Current() == screen)
            screen->OnTouch(touch, touching);

        graveyard_.clear();
    }

    graveyard_.clear();
    stack_.clear();
    running_ = false;
}

void ScreenManager::Push(std::unique_ptr<Screen> screen) {
    if (screen)
        stack_.push_back(std::move(screen));
}

void ScreenManager::Pop() {
    if (stack_.size() > 1) {
        graveyard_.push_back(std::move(stack_.back()));
        stack_.pop_back();
    }
}

Screen *ScreenManager::Current() const {
    return stack_.empty() ? nullptr : stack_.back().get();
}

bool ScreenManager::IsCurrent(const Screen *screen) const {
    return Current() == screen;
}

}
