#ifndef _3D_SHELL_VIRTUAL_KEYBOARD_H
#define _3D_SHELL_VIRTUAL_KEYBOARD_H

#include <3ds.h>

namespace VirtualKeyboard {
    typedef enum {
        LAYOUT_LOWER = 0,
        LAYOUT_UPPER = 1,
        LAYOUT_NUMERIC = 2,
        LAYOUT_SYMBOLS = 3
    } Layout;

    void Reset(void);
    Layout GetLayout(void);
    void SetLayout(Layout layout);
    void Display(void);
    void Control(u32 kDown);
}

#endif
