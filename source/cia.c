#include <stdlib.h>

#include "cia.h"
#include "progress_bar.h"
#include "utils.h"

static Result CIA_RemoveTitle(u64 titleID, FS_MediaType media)
{
	u32 count = 0;
	
	Result ret = AM_GetTitleCount(media, &count);
	if (R_FAILED(ret))
		return ret;
	
	u32 read = 0;
	u64 * titleIDs = malloc(count * sizeof(u64));
	
	ret = AM_GetTitleList(&read, media, count, titleIDs);
	if (R_FAILED(ret))
	{
		free(titleIDs);
		return ret;
	}
	
	for (unsigned int i = 0; i < read; i++) 
	{
		if (titleIDs[i] == titleID) 
		{
			ret = AM_DeleteAppTitle(media, titleID);
			if (R_FAILED(ret))
			{
				free(titleIDs);
				return ret;
			}
			break;
		}
	}
	
	free(titleIDs);
	
	if (R_FAILED(ret))
		return ret;
	
	return 0;
}

Result CIA_InstallTitle(const char * path, FS_MediaType media, bool update)
{
	u64 size = 0;
	u32 bytes;
	Handle ciaHandle, fileHandle;
	AM_TitleEntry title;
	u32 read = 0x1000;
	
	Result ret = 0;

	char * filename = Utils_Basename(path);

	if (R_FAILED(ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_ASCII, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0)))
		return ret;
	
	if (R_FAILED(ret = AM_GetCiaFileInfo(media, &title, fileHandle)))
		return ret;
	
	if (!update) // As long as we aren't updating 3DShell, remove the title before installing.
	{
		if (R_FAILED(ret = CIA_RemoveTitle(title.titleID, media)))
			return ret;
	}
	
	if (R_FAILED(ret = FSFILE_GetSize(fileHandle, &size)))
		return ret;
	
	if (R_FAILED(ret = AM_StartCiaInstall(media, &ciaHandle)))
		return ret;
	
	u8 * cia_buffer = malloc(read);
	
	for (u64 startSize = size; size != 0; size -= read) 
	{
		ProgressBar_DisplayProgress("Installing", filename, startSize, read);

		if (size < read) 
			read = size;
		FSFILE_Read(fileHandle, &bytes, startSize-size, cia_buffer, read);
		FSFILE_Write(ciaHandle, &bytes, startSize-size, cia_buffer, read, 0);
	}
	
	free(cia_buffer);

	if (R_FAILED(ret = AM_FinishCiaInstall(ciaHandle)))
		return ret;

	if (R_FAILED(ret = svcCloseHandle(fileHandle)))
		return ret;
	
	return 2;
}

static void CIA_LaunchTitle(u64 titleId, FS_MediaType mediaType)
{
	u8 param[0x300];
	u8 hmac[0x20];
	
	APT_PrepareToDoApplicationJump(0, titleId, mediaType);
	APT_DoApplicationJump(param, sizeof(param), hmac);
}
