#include <3ds.h>

#include "touch.h"

u16 Touch_GetX(void) {
    touchPosition pos;
    hidTouchRead(&pos);
    
    return pos.px;
}

u16 Touch_GetY(void) {
    touchPosition pos;
    hidTouchRead(&pos);
    
    return pos.py;
}
