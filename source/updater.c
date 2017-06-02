#include "fs.h"
#include "main.h"
#include "net.h"
#include "updater.h"

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