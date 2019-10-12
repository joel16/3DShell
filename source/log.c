#include <stdio.h>
#include <stdarg.h>

#include "fs.h"

static Handle log_handle;
static u64 offset = 0;

Result Log_OpenFileHande(void) {
	Result ret = 0;
	
	if (R_FAILED(ret = FS_OpenFile(&log_handle, sdmc_archive, "/3ds/3DShell/log.txt", FS_OPEN_WRITE, 0)))
		return ret;
		
	return 0;
}

Result Log_CloseFileHandle(void) {
	Result ret = 0;
	
	if (R_FAILED(ret = FSFILE_Close(log_handle)))
		return ret;
		
	return 0;
}

Result Log_Print(const char *s, ...) {
	Result ret = 0;
	u32 bytes_written = 0;
	char buf[256];
	va_list argptr;

	va_start(argptr, s);
	int length = vsnprintf(buf, sizeof(buf), s, argptr);
	va_end(argptr);

	if (R_FAILED(ret = FSFILE_Write(log_handle, &bytes_written, offset, buf, length, FS_WRITE_FLUSH)))
		return ret;

	offset += bytes_written;
	return 0;
}
