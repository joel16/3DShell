#ifndef _3D_SHELL_PANEL_APP_H
#define _3D_SHELL_PANEL_APP_H

#include <functional>

#include <container_control.h>

namespace GUI {

/* Base for applications that live inside the desktop's bottom panel
 * (below the touch bar). The desktop owns the instance, routes input
 * and calls Draw()/OnFrame() every frame. Close() fires the Closed
 * event so the desktop removes it (deferred destruction). */
class PanelApp : public UI::ContainerControl {
public:
    PanelApp() {
        bounds_ = UI::Rect{0.0f, 20.0f, 320.0f, 220.0f};
    }

    virtual void OnFrame(u64 delta_time) { (void)delta_time; }

    void Close() {
        if (Closed)
            Closed();
    }

    std::function<void()> Closed;
};

}

#endif
