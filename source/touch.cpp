#include "touch.h"

namespace Touch {
    static touchPosition position;
    static touchPosition prev;

    void Update(void) {
        prev = position;
        hidTouchRead(&position);
    }

    u16 GetX(void) {
        return position.px;
    }

    u16 GetY(void) {
        return position.py;
    }

    bool Rect(u16 x, u16 y, u16 w, u16 h) {
        const u16 px = position.px;
        const u16 py = position.py;
        return (px >= x && px <= (x + w) && py >= y && py <= (y + h));
    }

    bool IsTouching(void) {
        return ((position.px != 0) || (position.py != 0));
    }

    bool JustPressed(void) {
        const bool prevTouch = (prev.px != 0 || prev.py != 0);
        const bool currTouch = (position.px != 0 || position.py != 0);
        return (!prevTouch && currTouch);
    }

    bool JustReleased(void) {
        const bool prevTouch = (prev.px != 0 || prev.py != 0);
        const bool currTouch = (position.px != 0 || position.py != 0);
        return (prevTouch && !currTouch);
    }
}
