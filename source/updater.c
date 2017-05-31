#include "fs.h"
#include "main.h"
#include "updater.h"

Result downloadFile(const char * url, const char * path)
{
	if (url == NULL) // Invalid URL
		return 1;
	
	if (path == NULL) // Dir path does not exist
		return 1;
	
	httpcContext context;
	Result ret = 0;
	u32 statuscode = 0;
	u32 contentsize = 0;
	u8 *buf;
	
	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 0);
	if (ret != 0) // httpcOpenContext failed
		return ret;
	
	ret = httpcAddRequestHeaderField(&context, "Updater-Agent", "3DShell");
	if (ret != 0) // httpcAddRequestHeaderField failed
		return ret;
	
	ret = httpcSetSSLOpt(&context, 1<<9);
	if (ret != 0) // httpcSetSSLOpt failed
		return ret;
	
	ret = httpcBeginRequest(&context);
	if(ret != 0) // httpcBeginRequest failed
		return ret;
	
	ret = httpcGetResponseStatusCode(&context, &statuscode);
	if (ret != 0) // httpcGetResponseStatusCode failed
	{
		httpcCloseContext(&context);
		return ret;
	}
	
	if (statuscode != 200)
	{
		if (statuscode >= 300 && statuscode < 400) 
		{
			char newUrl[1024];
			ret = httpcGetResponseHeader(&context, (char*)"Location", newUrl, 1024);
			if (ret != 0)
			
				return ret;
			
			httpcCloseContext(&context);
			ret = downloadFile(newUrl, path);
			return ret;
		}
		else
		{
			httpcCloseContext(&context);
			return -1;
		}
	}
	
	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret != 0) // httpcGetDownloadSizeState failed
	{
		httpcCloseContext(&context);
		return ret;
	}
	
	buf = (u8*)malloc(contentsize);
	if (buf == NULL) // Malloc failed
		return -2;
	
	memset(buf, 0, contentsize);
	
	ret = httpcDownloadData(&context, buf, contentsize, NULL);
	if(ret != 0) // httpcDownloadData failed
	{
		free(buf);
		httpcCloseContext(&context);
		return -3;
	}
	
	remove(path);
	
	FILE * file = fopen(path, "wb");
	fwrite(buf, 1, contentsize, file); // Success
	fclose(file);
	
	free(buf);
	httpcCloseContext(&context);
	
	return 0;
}

char * checkForUpdate()
{
	int currVer = 0, updateVer = 0;
	static char str[20];
	
	Result ret = downloadFile("https://github.com/3DShell-Bot/Updater/releases/download/UPDATER/version.txt", "/3ds/3DShell/version.txt");
	
	currVer = VERSION_MAJOR + VERSION_MINOR; // A shitty way to get updates I know - I plan to change this later.
	
	if (ret == 0)
	{
		FILE * file = fopen("/3ds/3DShell/version.txt", "r");
		fscanf(file, "%d", &updateVer);
		fclose(file);
	}
	
	if (updateVer > currVer)
		strcpy(str, "Update found");
	else
		strcpy(str, "No updates found.");
	
	return str;
}

void downloadUpdate()
{
	Result ret = downloadFile("https://github.com/3DShell-Bot/Updater/releases/download/UPDATER/3DShell.cia", "/3ds/3DShell/3DShell.cia");
	
	if (ret == 0)
	{
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Downloaded update")) / 2), 60, RGBA8(251, 251, 251, 255), 11, "Downloaded update");
		wait(100000000);
		installUpdate();
	}
}

Handle openFileHandle(const char *path, u32 openFlags) 
{
	Handle fileHandle = 0;
  
	FS_Path filePath = fsMakePath(PATH_ASCII, path);
	FSUSER_OpenFile(&fileHandle, sdmcArchive, filePath, openFlags, 0);
	
	return(fileHandle);
}

void closeFileHandle(const Handle handle) 
{
	FSFILE_Close(handle);
}

void installCIA(const u8* ciaData, const size_t ciaSize) 
{
	Handle handle;
	AM_QueryAvailableExternalTitleDatabase(NULL);
	AM_StartCiaInstall(MEDIATYPE_SD, &handle);
	FSFILE_Write(handle, NULL, 0, ciaData, (u32)ciaSize, 0);
	AM_FinishCiaInstall(handle);
	//AM_CancelCIAInstall(handle);	// Installation aborted.
}

void installUpdate()
{
	//installCIA(&ciaData, sizeof(ciaData));
	
	sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Update completed")) / 2), 80, RGBA8(251, 251, 251, 255), 11, "Update completed");
	
	wait(200000000);
	
	mainMenu(CLEAR);
}