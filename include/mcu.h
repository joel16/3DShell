#ifndef MCU_H
#define MCU_H

#include <3ds.h>

Handle mcuhwcHandle;

Result mcuInit(void);
Result mcuExit(void);
Result mcuGetBatteryLevel(u8* out);

#endif