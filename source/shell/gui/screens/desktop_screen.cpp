#include "desktop_screen.h"

#include "app_drawer_panel.h"
#include "file_explorer_screen.h"
#include "hbloader.h"
#include "log.h"
#include "net_loader_panel.h"
#include "settings_panel.h"
#include "touch.h"

#include "apps.h"

namespace GUI {

    static constexpr u32 DESKTOP_BG_DARK = C2D_Color32(39, 43, 58, 255);
    static constexpr u32 DESKTOP_BG_LIGHT = C2D_Color32(207, 216, 220, 255);

DesktopScreen::DesktopScreen() {
    SetTopBackground(cfg.dark_theme? DESKTOP_BG_DARK : DESKTOP_BG_LIGHT);
    SetBottomBackground(cfg.dark_theme? DESKTOP_BG_DARK : DESKTOP_BG_LIGHT);

    touch_bar_.SetSection(TouchBar::SECTION_NONE);

    touch_bar_.AddButton(TouchBar::ICON_HOME, false).Click = [this]() {
        auto drawer = std::make_unique<AppDrawerPanel>();

        /* Fullscreen apps push onto the screen stack; panel apps replace
         * the current bottom panel. */
        drawer->LaunchScreen = [this](std::unique_ptr<UI::Screen> screen) {
            ClosePanel();
            if (screen)
                UI::ScreenManager::Instance().Push(std::move(screen));
        };

        drawer->SwitchPanel = [this](Apps::Id id, std::unique_ptr<GUI::PanelApp> panel) {
            PanelType type = PanelType::Custom;
            if (id == Apps::AppSettings)
                type = PanelType::Settings;
            else if (id == Apps::AppNetLoader)
                type = PanelType::NetLoader;
            TogglePanel(type, std::move(panel));
        };

        TogglePanel(PanelType::Drawer, std::move(drawer));
    };

    /* The right side of the bar is reserved space for the active app's
     * own actions. Settings and NetLoader are reached from the drawer. */
}

void DesktopScreen::OnFrame(u64 delta_time) {
    dying_panel_.reset();

    if (HBLoader::PendingExit())
        UI::ScreenManager::Instance().Exit();

    if (panel_app_)
        panel_app_->OnFrame(delta_time);
}

void DesktopScreen::DrawTop() {
    status_bar_.Draw();

    /* Placeholder desktop title until the wallpaper lands. */
    float text_width = 0.0f, text_height = 0.0f;
    C2D::GetTextSize(0.7f, &text_width, &text_height, "3DShell");
    C2D::Text(((400.0f - text_width) / 2.0f), ((240.0f - text_height) / 2.0f), 0.7f,
        cfg.dark_theme? WHITE : BLACK, "3DShell");
}

void DesktopScreen::DrawBottom() {
    touch_bar_.Draw();

    if (panel_app_)
        panel_app_->Draw();
}

void DesktopScreen::OnKeyHeld(u32 k_held) {
    (void)k_held;
    Touch::Update();
}

void DesktopScreen::OnTouch(const touchPosition &touch, bool down) {
    bool in_bar = down? (touch.py < 20) : bar_touching_;

    if (in_bar) {
        touch_bar_.OnTouch(touch, down);
        bar_touching_ = down;
        return;
    }

    bar_touching_ = false;

    /* The rest of the panel belongs to the open panel app; releases are
     * forwarded too so its buttons complete their click. */
    bool in_panel = down? (touch.py >= 20) : panel_touching_;

    if (in_panel) {
        if (panel_app_)
            panel_app_->OnTouch(touch, down);
        panel_touching_ = down;
    }
}

void DesktopScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    if (panel_app_)
        panel_app_->OnKeyDown(k_down);
}

void DesktopScreen::OpenExplorer() {
    auto explorer = std::make_unique<FileExplorerScreen>();
    if (R_SUCCEEDED(explorer->Init())) {
        ClosePanel();
        UI::ScreenManager::Instance().Push(std::move(explorer));
    }
}

void DesktopScreen::TogglePanel(PanelType type, std::unique_ptr<PanelApp> app) {
    if (panel_type_ == type) {
        ClosePanel();
        return;
    }

    ClosePanel();

    app->Closed = [this]() {
        ClosePanel();
    };

    panel_app_ = std::move(app);
    panel_type_ = type;
}

void DesktopScreen::ClosePanel() {
    if (panel_app_) {
        dying_panel_ = std::move(panel_app_);
        panel_type_ = PanelType::None;
    }
}

}
