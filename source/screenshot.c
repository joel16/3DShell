#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "fs.h"
#include "menu_error.h"
#include "screenshot.h"

static Result Screenshot_GenerateScreenshot(const char *path) {
	int x = 0, y = 0;
	size_t size = 0x36;
	Result ret = 0;

	// Get top/bottom framebuffers
	u8 *top_framebuf = gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL);
	u8 *bottom_framebuf = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

	u8 *buf = NULL;
	buf = linearAlloc(size + 576000);
	memset(buf, 0, size + 576000);
	buf[size + 576000] = 0;

	*(u16*)&buf[0x0] = 0x4D42;
	*(u32*)&buf[0x2] = size + 576000;
	*(u32*)&buf[0xA] = size;
	*(u32*)&buf[0xE] = 0x28;
	*(u32*)&buf[0x12] = 400;
	*(u32*)&buf[0x16] = 480;
	*(u32*)&buf[0x1A] = 0x00180001;
	*(u32*)&buf[0x22] = 576000;

	// Generate top left
	u8 *framebuf = bottom_framebuf;

	for (y = 0; y < 240; y++) {
		for (x = 0; x < 400; x++) {
			int si = ((239 - y) + (x * 240)) * 3;
			int di = size + (x + ((479 - y) * 400)) * 3;
			buf[di++] = framebuf[si++];
			buf[di++] = framebuf[si++];
			buf[di++] = framebuf[si++];
		}
	}

	// Generate bottom right
	framebuf = top_framebuf;

	for (y = 0; y < 240; y++) {
		for (x = 0; x < 320; x++) {
			int si = ((239 - y) + (x * 240)) * 3;
			int di = size + ((x+40) + ((239 - y) * 400)) * 3;
			buf[di++] = framebuf[si++];
			buf[di++] = framebuf[si++];
			buf[di++] = framebuf[si++];
		}

		// Make adjustments for the smaller width
		for (x = 0; x < 40; x++) {
			int di = size + (x + ((239 - y) * 400)) * 3;
			buf[di++] = 0;
			buf[di++] = 0;
			buf[di++] = 0;
		}

		for (x = 360; x < 400; x++) {
			int di = size + (x + ((239 - y) * 400)) * 3;
			buf[di++] = 0;
			buf[di++] = 0;
			buf[di++] = 0;
		}
	}

	if (R_FAILED(ret = FS_Write(sdmc_archive, path, (u32 *)buf, size + 576000))) {
		Menu_DisplayError("FS_Write screenshot failed:", ret);
		free(buf);
	}

	linearFree(buf);
	return 0;
}

static void Screenshot_GenerateFilename(int count, char *file_name) {
	time_t t = time(0);
	int day = localtime(&t)->tm_mday;
	int month = localtime(&t)->tm_mon + 1;
	int year = localtime(&t)->tm_year + 1900;
	
	if (!(FS_DirExists(sdmc_archive, "/screenshots/")))
		FS_MakeDir(sdmc_archive, "/screenshots");
		
	sprintf(file_name, "/screenshots/Screenshot_%02d%02d%02d-%i.bmp", year, month, day, count);
}

void Screenshot_Capture(void) {
	int num = 0;
	static char file_name[256];
	sprintf(file_name, "%s", "screenshot");
	Screenshot_GenerateFilename(num, file_name);

	while (FS_FileExists(sdmc_archive, file_name)) {
		num++;
		Screenshot_GenerateFilename(num, file_name);
	}

	Screenshot_GenerateScreenshot(file_name);
	num++;
}
