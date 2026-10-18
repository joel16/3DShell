#ifndef _3D_SHELL_UI_CONTAINER_CONTROL_H
#define _3D_SHELL_UI_CONTAINER_CONTROL_H

#include <memory>
#include <vector>

#include "control.h"

namespace UI {

class ContainerControl : public Control {
public:
    void Add(std::unique_ptr<Control> control);
    void Clear();

    void Draw() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    std::vector<std::unique_ptr<Control>> controls_;
    Control *touch_capture_ = nullptr;
    bool touch_was_down_ = false;
};

}

#endif
