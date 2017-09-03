#ifndef MCU_H
#define MCU_H

#include <3ds.h>

Result mcuInit(void);
Result mcuExit(void);
Result MCU_GetBatteryLevel(u8 * out);

#endif