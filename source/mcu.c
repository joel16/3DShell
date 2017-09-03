#include "mcu.h"

static Handle mcuHandle;

Result mcuInit(void)
{
    return srvGetServiceHandle(&mcuHandle, "mcu::HWC");
}

Result mcuExit(void)
{
    return svcCloseHandle(mcuHandle);
}

Result MCU_GetBatteryLevel(u8 * out)
{
	Result ret = 0;
	u32 * cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x05, 0, 0); // 0x00050000

	if (R_FAILED(ret = svcSendSyncRequest(mcuHandle)))
		return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}