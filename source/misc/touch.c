#include <3ds.h>

#include "touch.h"

u16 touchGetX(void)
{
	touchPosition pos;
	hidTouchRead(&pos);
	
	return pos.px;
}

u16 touchGetY(void)
{
	touchPosition pos;
	hidTouchRead(&pos);
	
	return pos.py;
}