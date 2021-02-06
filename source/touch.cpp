#include "touch.h"

namespace Touch {
    static touchPosition position;

    void Update(void) {
        hidTouchRead(&position);
    }

    u16 GetX(void) {
        return position.px;
    }
    
    u16 GetY(void) {
        return position.py;
    }

    bool Rect(u16 x, u16 y, u16 w, u16 h) {
        return ((Touch::GetX() >= (x) && Touch::GetX() <= (w)) && (Touch::GetY() >= (y) && Touch::GetY() <= (h)));
    }
}
