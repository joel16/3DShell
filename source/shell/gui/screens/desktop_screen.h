#ifndef _3D_SHELL_DESKTOP_SCREEN_H
#define _3D_SHELL_DESKTOP_SCREEN_H

#include <memory>

#include <panel_app.h>
#include <status_bar.h>
#include <touch_bar.h>
#include "screen.h"
#include "screen_manager.h"

namespace GUI {

/* The shell home screen: wallpaper on the top display; on the bottom
 * display the touch bar plus one panel app at a time (app drawer,
 * settings, netloader) rendered below the bar. */
class DesktopScreen : public UI::Screen {
public:
    DesktopScreen();

    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    enum class PanelType {
        None,
        Drawer,
        Settings,
        NetLoader,
        Custom
    };

    void OpenExplorer();
    void TogglePanel(PanelType type, std::unique_ptr<PanelApp> app);
    void ClosePanel();

    StatusBar status_bar_;
    TouchBar touch_bar_;
    bool bar_touching_ = false;
    bool panel_touching_ = false;

    std::unique_ptr<PanelApp> panel_app_;
    PanelType panel_type_ = PanelType::None;
    std::unique_ptr<PanelApp> dying_panel_;
};

}

#endif
