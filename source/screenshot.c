#include "fs.h"
#include "screenshot.h"

static int num = 0;

static int generateScreenshot(const char * path)
{	
	int x = 0, y = 0;
	Handle handle;
	u32 bytesWritten = 0;
	u64 offset = 0;
	size_t size = 0x36;
	
	// Get top/bottom framebuffers
	u8 * gfxBottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL);
	u8 * gfxTopLeft = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	
	// Open file for writing screenshot
	fsOpen(&handle, path, (FS_OPEN_CREATE | FS_OPEN_WRITE));
	
	// Some 
	u8 * buffer = (u8*)malloc(size + 576000);
	memset(buffer, 0, size + 576000);
	buffer[size + 576000] = 0;
	
	FSFILE_SetSize(handle, (u16)(size + 576000));
	
	*(u16*)&buffer[0x0] = 0x4D42;
	*(u32*)&buffer[0x2] = size + 576000;
	*(u32*)&buffer[0xA] = size;
	*(u32*)&buffer[0xE] = 0x28;
	*(u32*)&buffer[0x12] = 400;
	*(u32*)&buffer[0x16] = 480;
	*(u32*)&buffer[0x1A] = 0x00180001;
	*(u32*)&buffer[0x22] = 576000;
	
	// Generate top left
	u8* framebuf = gfxTopLeft;
	
	for (y = 0; y < 240; y++)
	{
		for (x = 0; x < 400; x++)
		{
			int si = ((239 - y) + (x * 240)) * 3;
			int di = size + (x + ((479 - y) * 400)) * 3;
			buffer[di++] = framebuf[si++];
			buffer[di++] = framebuf[si++];
			buffer[di++] = framebuf[si++];
		}	
	}
	
	// Generate bottom right
	framebuf = gfxBottom;
	
	for (y = 0; y < 240; y++)
	{
		for (x = 0; x < 320; x++)
		{
			int si = ((239 - y) + (x * 240)) * 3;
			int di = size + ((x+40) + ((239 - y) * 400)) * 3;
			buffer[di++] = framebuf[si++];
			buffer[di++] = framebuf[si++];
			buffer[di++] = framebuf[si++];
		}
		
		// Make adjustments for the smaller width
		for (x = 0; x < 40; x++)
		{
			int di = size + (x + ((239 - y) * 400)) * 3;
			buffer[di++] = 0;
			buffer[di++] = 0;
			buffer[di++] = 0;
		}
		
		for (x = 360; x < 400; x++)
		{
			int di = size + (x + ((239 - y) * 400)) * 3;
			buffer[di++] = 0;
			buffer[di++] = 0;
			buffer[di++] = 0;
		}
	}
	
	FSFILE_Write(handle, &bytesWritten, offset, (u32 *)buffer, size + 576000, 0x10001);
	FSFILE_Close(handle);
	free(buffer);
	
	return 0;
}

static void generateScreenshotFileName(int number, char *fileName, const char *ext)
{
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	int num = number;
	int day = timeStruct->tm_mday;
	int month = timeStruct->tm_mon + 1;
	int year = timeStruct->tm_year + 1900;
	
	if (!(dirExists(fsArchive, "/screenshots/")))
		makeDir(fsArchive, "/screenshots");

	sprintf(fileName, "/screenshots/Screenshot_%02d%02d%02d-%i%s", year, month, day, num, ext);
}

void captureScreenshot(void)
{	
	static char filename[256];
	
	sprintf(filename, "%s", "screenshot"); 

	generateScreenshotFileName(num, filename, ".bmp"); 
	
	while (fileExists(fsArchive, filename))
	{
		num++;
		generateScreenshotFileName(num, filename, ".bmp");
	}
	
	generateScreenshot(filename);
	num++;
}