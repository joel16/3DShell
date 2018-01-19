#ifndef TOUCH_H
#define TOUCH_H

#define touchInRect(x1, y1, x2, y2) ((touchGetX() >= (x1) && touchGetX() <= (x2)) && (touchGetY() >= (y1) && touchGetY() <= (y2)))

u16 touchGetX(void);
u16 touchGetY(void);

#endif