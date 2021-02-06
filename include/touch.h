#ifndef _3D_SHELL_TOUCH_H
#define _3D_SHELL_TOUCH_H

#include <3ds.h>

namespace Touch {
    void Update(void);
    u16 GetX(void);
    u16 GetY(void);
    bool Rect(u16 x, u16 y, u16 w, u16 h);
}

#endif
