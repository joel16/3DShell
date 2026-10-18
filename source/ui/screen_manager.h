#ifndef _3D_SHELL_UI_SCREEN_MANAGER_H
#define _3D_SHELL_UI_SCREEN_MANAGER_H

#include <memory>
#include <vector>

#include <3ds.h>

#include "screen.h"

namespace UI {

class ScreenManager {
public:
    static ScreenManager &Instance();

    void Run(std::unique_ptr<Screen> root);
    void Push(std::unique_ptr<Screen> screen);
    void Pop();
    void Exit() { running_ = false; }

    bool IsCurrent(const Screen *screen) const;

    u64 DeltaTime() const { return delta_time_; }

private:
    ScreenManager() = default;

    Screen *Current() const;

    std::vector<std::unique_ptr<Screen>> stack_;
    std::vector<std::unique_ptr<Screen>> graveyard_;
    bool running_ = false;
    u64 delta_time_ = 0;
};

}

#endif
