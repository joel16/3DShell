#ifndef ACT_H
#define ACT_H

#include <3ds.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Handle actHandle;
int actRefCount;

u32 *act_shareMemAddr;
u32 act_shareMemSize;
Handle act_shareMemHandle;

Result actInit(u32 sdkVer, u32 sharedMemSize);
void actExit(void);
Result ACT_Initialize(u32 sdkVer, void *addr, u32 memSize);
Result ACT_GetAccountInfo(void *buffer, u32 size, u32 blkId);
char * getNNID(void);

#endif