#include "mcu.h"

Result mcuInit(void)
{
    return srvGetServiceHandle(&mcuhwcHandle, "mcu::HWC");
}

Result mcuExit(void)
{
    return svcCloseHandle(mcuhwcHandle);
}

Result mcuGetBatteryLevel(u8* out)
{
    u32* ipc = getThreadCommandBuffer();
    ipc[0] = 0x50000;
    Result ret = svcSendSyncRequest(mcuhwcHandle);
    if(ret < 0) return ret;
	*out = ipc[2];
    return ipc[1];
}