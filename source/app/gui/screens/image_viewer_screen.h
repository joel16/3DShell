#ifndef _3D_SHELL_IMAGE_VIEWER_SCREEN_H
#define _3D_SHELL_IMAGE_VIEWER_SCREEN_H

#include <3ds.h>
#include <citro2d.h>

#include "gui.h"
#include "screen.h"
#include "screen_manager.h"

namespace GUI {

class ImageViewerScreen : public UI::Screen {
public:
    ImageViewerScreen(MenuItem *item);

    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;

private:
    enum ImageStates {
        DIMENSION_DEFAULT = 0,
        DIMENSION_NINTENDO_SCREENSHOT = 1,
        DIMENSION_DUAL_SCREEN = 2
    };

    std::string EntryNameUtf8(u32 index);
    bool IsImageEntry(u32 index);
    bool FindNeighborImage(int dir, u32 *out_index);
    bool SwitchImage(int dir);
    bool DrawImage(C2D_Image image, float x, float y, float start, float end, float w, float h, float zoom_factor);
    void DrawImageTop();
    void DrawImageBottom();
    void Control(u32 k_down);

    MenuItem *item_;
    u64 delta_time_ = 0;
    u32 k_held_ = 0;
    bool properties_ = false;
    float ok_height_ = 0.0f, ok_width_ = 0.0f, scale_factor_ = 0.0f, width_ = 0.0f, height_ = 0.0f, zoom_factor_ = 1.0f;
    int pos_x_ = 0, pos_y_ = 0;
    ImageStates image_state_ = DIMENSION_DEFAULT;
};

}

#endif
