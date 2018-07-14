#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "utils.h"

Result FS_OpenArchive(FS_Archive *archive, FS_ArchiveID archiveID)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_OpenArchive(archive, archiveID, fsMakePath(PATH_EMPTY, ""))))
		return ret;

	return 0;
}

Result FS_CloseArchive(FS_Archive archive)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_CloseArchive(archive)))
		return ret;

	return 0;
}

Result FS_MakeDir(FS_Archive archive, const char *path)
{	
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_CreateDirectory(archive, fsMakePath(PATH_ASCII, path), 0)))
		return ret;
	
	return 0;
}

Result FS_RecursiveMakeDir(FS_Archive archive, const char *dir) 
{
	Result ret = 0;
	char buf[256];
	char *p = NULL;
	size_t len;

	snprintf(buf, sizeof(buf), "%s",dir);
	len = strlen(buf);

	if (buf[len - 1] == '/')
		buf[len - 1] = 0;

	for (p = buf + 1; *p; p++)
	{
		if (*p == '/') 
		{
			*p = 0;

			ret = FS_MakeDir(archive, buf);
			
			*p = '/';
		}
		
		ret = FS_MakeDir(archive, buf);
	}
	
	return ret;
}

bool FS_FileExists(FS_Archive archive, const char *path)
{
	Handle handle;

	if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0)))
		return false;

	if (R_FAILED(FSFILE_Close(handle)))
		return false;

	return true;
}

bool FS_DirExists(FS_Archive archive, const char *path)
{
	Handle handle;

	if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_ASCII, path))))
		return false;

	if (R_FAILED(FSDIR_Close(handle)))
		return false;

	return true;
}

/*char *FS_GetFileModifiedTime(char *path)
{
	static char timeStr[20];
	u64 mtime = 0;

	if (R_SUCCEEDED(sdmc_getmtime(path, &mtime)))
	{
		time_t mt = mtime;
		struct tm *timeStruct = gmtime(&mt);

		int hours = timeStruct->tm_hour;
		int minutes = timeStruct->tm_min;
		bool amOrPm = false;

		if (hours < 12)
			amOrPm = true;
		if (hours == 0)
			hours = 12;
		else if (hours > 12)
			hours = hours - 12;

		int day = timeStruct->tm_mday;
		int month = timeStruct->tm_mon + 1; // January being 0
		int year = timeStruct->tm_year + 1900;

		if ((Utils_GetRegion() == CFG_REGION_JPN) || (Utils_GetRegion() == CFG_REGION_CHN) || (Utils_GetRegion() == CFG_REGION_KOR) || (Utils_GetRegion() == CFG_REGION_TWN))
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i %s", year, month, day, hours, minutes, amOrPm ? "AM" : "PM");
		else if (Utils_GetRegion() == CFG_REGION_USA) // This is the worst one
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i %s", month, day, year, hours, minutes, amOrPm ? "AM" : "PM");
		else 
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i %s", day, month, year, hours, minutes, amOrPm ? "AM" : "PM");
	}

	return timeStr;
}*/

Result FS_GetFileSize(FS_Archive archive, const char *path, u64 *size)
{
	Handle handle;
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0)))
		return ret;

	if (R_FAILED(ret = FSFILE_GetSize(handle, size)))
		return ret;

	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;
}

u64 FS_GetFreeStorage(FS_SystemMediaType mediaType)
{
	FS_ArchiveResource	resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, mediaType)))
		return (((u64) resource.freeClusters *(u64) resource.clusterSize));

	return 0;
}

u64 FS_GetTotalStorage(FS_SystemMediaType mediaType)
{
	FS_ArchiveResource	resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, mediaType)))
		return (((u64) resource.totalClusters *(u64) resource.clusterSize));

	return 0;
}

u64 FS_GetUsedStorage(FS_SystemMediaType mediaType)
{
	FS_ArchiveResource	resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, mediaType)))
		return ((((u64) resource.totalClusters *(u64) resource.clusterSize)) - (((u64) resource.freeClusters *(u64) resource.clusterSize)));

	return 0;
}

Result FS_Remove(FS_Archive archive, const char *filename)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_DeleteFile(archive, fsMakePath(PATH_ASCII, filename))))
		return ret;
	
	return 0;
}

Result FS_Rmdir(FS_Archive archive, const char *path)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_DeleteDirectory(archive, fsMakePath(PATH_ASCII, path))))
		return ret;
	
	return 0;
}

Result FS_RmdirRecursive(FS_Archive archive, const char *path)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_DeleteDirectoryRecursively(archive, fsMakePath(PATH_ASCII, path))))
		return ret;
	
	return 0;
}

Result FS_RenameFile(FS_Archive archive, const char *old_filename, const char *new_filename)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_RenameFile(archive, fsMakePath(PATH_ASCII, old_filename), archive, fsMakePath(PATH_ASCII, new_filename))))
		return ret;
	
	return 0;
}

Result FS_RenameDir(FS_Archive archive, const char *old_filename, const char *new_filename)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_RenameDirectory(archive, fsMakePath(PATH_ASCII, old_filename), archive, fsMakePath(PATH_ASCII, new_filename))))
		return ret;
	
	return 0;
}

Result FS_Open(Handle *handle, FS_Archive archive, const char *path, u32 flags)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_OpenFile(handle, archive, fsMakePath(PATH_ASCII, path), flags, 0)))
		return ret;
	
	return 0;
}

Result FS_Read(FS_Archive archive, const char *path, u64 size, char *buf)
{
	Handle handle;
	Result ret = 0;

	u32 bytesread = 0;

	if (R_FAILED(ret = FS_Open(&handle, archive, path, FS_OPEN_READ)))
		return ret;
	
	if (R_FAILED(ret = FSFILE_Read(handle, &bytesread, 0, (u32 *)buf, size)))
		return ret;

	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;

	return 0;
}

Result FS_Write(FS_Archive archive, const char *path, const char *buf)
{
	Handle handle;
	Result ret = 0;
	
	u32 len = strlen(buf);
	u64 size = 0;
	u32 bytesWritten = 0;

	if (FS_FileExists(archive, path))
		FS_Remove(archive, path);

	if (R_FAILED(ret = FS_Open(&handle, archive, path, (FS_OPEN_WRITE | FS_OPEN_CREATE))))
		return ret;

	if (R_FAILED(ret = FSFILE_GetSize(handle, &size)))
		return ret;

	if (R_FAILED(ret = FSFILE_SetSize(handle, size + len)))
		return ret;

	if (R_FAILED(ret = FSFILE_Write(handle, &bytesWritten, size, buf, len, FS_WRITE_FLUSH)))
		return ret;

	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;

	return 0;
}