#include "cia.h"

Result removeTitle(u64 titleID, FS_MediaType media)
{
	u32 count = 0;
	Result ret = AM_GetTitleCount(media, &count);
	
	if (ret) 
		return ret;
	
	u32 read = 0;
	u64 * titleIDs = malloc(count * sizeof(u64));
	
	ret = AM_GetTitleList(&read, media, count, titleIDs);
	
	if (ret)
		return ret;
	
	for (unsigned int i = 0; i < read; i++) 
	{
		if (titleIDs[i] == titleID) 
		{
			ret = AM_DeleteAppTitle(media, titleID);
			break;
		}
	}
	
	free(titleIDs);
	
	if (ret) 
		return ret;
	
	return 0;
}

Result installCIA(const char * path, FS_MediaType media, bool update)
{
	u64 size = 0;
	u32 bytes;
	Handle ciaHandle, fileHandle;
	AM_TitleEntry title;
	u32 read = 0x1000;
	
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_ASCII, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
	
	amInit();
	AM_QueryAvailableExternalTitleDatabase(NULL);
	
	ret = AM_GetCiaFileInfo(media, &title, fileHandle);
	
	if (!update) // As long as we aren't updating 3DShell, remove the title before installing.
	{
		ret = removeTitle(title.titleID, media);
		if (ret) 
			return ret;
	}
	
	ret = FSFILE_GetSize(fileHandle, &size);
	
	ret = AM_StartCiaInstall(media, &ciaHandle);
	
	u8 * cia_buffer = malloc(read);
	
	for (u64 startSize = size; size != 0; size -= read) 
	{
		if (size < read) 
			read = size;
		FSFILE_Read(fileHandle, &bytes, startSize-size, cia_buffer, read);
		FSFILE_Write(ciaHandle, &bytes, startSize-size, cia_buffer, read, 0);
	}
	
	free(cia_buffer);
	
	ret = AM_FinishCiaInstall(ciaHandle);

	ret = svcCloseHandle(fileHandle);
	
	if (ret)
		return ret;
		
	amExit();
	
	return 0;
}