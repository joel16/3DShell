#ifndef _3D_SHELL_NET_LOADER_PANEL_H
#define _3D_SHELL_NET_LOADER_PANEL_H

#include "panel_app.h"
#include "netloader.h"

namespace GUI {

/* NetLoader as a panel app: spawns the worker thread (NetLoader::Start)
 * and draws a live progress bar from the shared state every frame, like
 * hbmenu's UI thread. B cancels; the panel closes when the worker ends. */
class NetLoaderPanel : public GUI::PanelApp {
public:
    void OnFrame(u64 delta_time) override;
    void Draw() override;
    void OnKeyDown(u32 k_down) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    void DrawProgressBar(float y, s64 offset, s64 size);

    bool started_ = false;
    bool touching_ = false;
    UI::Rect back_rect_;
};

}

#endif
