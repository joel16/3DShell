#pragma once

#include <3ds.h>

namespace Touch {
    void Update(void);
    u16 GetX(void);
    u16 GetY(void);
    bool Rect(u16 x, u16 y, u16 w, u16 h);
    bool IsTouching(void);
    bool JustPressed(void);
    bool JustReleased(void);
}
