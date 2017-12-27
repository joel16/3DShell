#include <stdio.h>
#include <string.h>

#include "file/dirlist.h"
#include "file/fs.h"
#include "utils.h"


Result openArchive(FS_Archive * archive, FS_ArchiveID archiveID)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_OpenArchive(archive, archiveID, fsMakePath(PATH_EMPTY, ""))))
		return ret;

	return 0;
}

Result closeArchive(FS_Archive archive)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_CloseArchive(archive)))
		return ret;

	return 0;
}

Result makeDir(FS_Archive archive, const char * path)
{	
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_CreateDirectory(archive, fsMakePath(PATH_ASCII, path), 0)))
		return ret;
	
	return 0;
}

void recursiveMakeDir(FS_Archive archive, const char * dir) 
{
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s",dir);
	len = strlen(tmp);

	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for (p = tmp + 1; *p; p++)
	{
		if (*p == '/') 
		{
			*p = 0;
			makeDir(archive, tmp);
			*p = '/';
		}
		makeDir(archive, tmp);
	}
}

bool fileExists(FS_Archive archive, const char * path)
{
	Handle handle;

	if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0)))
		return false;

	if (R_FAILED(FSFILE_Close(handle)))
		return false;

	return true;
}

bool dirExists(FS_Archive archive, const char * path)
{
	Handle handle;

	if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_ASCII, path))))
		return false;

	if (R_FAILED(FSDIR_Close(handle)))
		return false;

	return true;
}

char * getFileModifiedTime(char * path)
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

		if ((getRegion() == CFG_REGION_JPN) || (getRegion() == CFG_REGION_CHN) || (getRegion() == CFG_REGION_KOR) || (getRegion() == CFG_REGION_TWN))
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i %s", year, month, day, hours, minutes, amOrPm ? "AM" : "PM");
		else if (getRegion() == CFG_REGION_USA) // This is the worst one
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i %s", month, day, year, hours, minutes, amOrPm ? "AM" : "PM");
		else 
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i %s", day, month, year, hours, minutes, amOrPm ? "AM" : "PM");
	}

	return timeStr;
}

u64 getFileSize(FS_Archive archive, const char * path)
{
	Handle handle;
	Result ret = 0;
	u64 st_size = 0;

	if (R_FAILED(ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0)))
		return ret;

	if (R_FAILED(ret = FSFILE_GetSize(handle, &st_size)))
		return ret;

	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;

	return st_size;
}

Result fsRemove(FS_Archive archive, const char * filename)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_DeleteFile(archive, fsMakePath(PATH_ASCII, filename))))
		return ret;
	
	return 0;
}

Result fsRmdir(FS_Archive archive, const char * path)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_DeleteDirectory(archive, fsMakePath(PATH_ASCII, path))))
		return ret;
	
	return 0;
}

Result fsRmdirRecursive(FS_Archive archive, const char * path)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_DeleteDirectoryRecursively(archive, fsMakePath(PATH_ASCII, path))))
		return ret;
	
	return 0;
}

Result fsRenameFile(FS_Archive archive, const char * old_filename, const char * new_filename)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_RenameFile(archive, fsMakePath(PATH_ASCII, old_filename), archive, fsMakePath(PATH_ASCII, new_filename))))
		return ret;
	
	return 0;
}

Result fsRenameDir(FS_Archive archive, const char * old_filename, const char * new_filename)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_RenameDirectory(archive, fsMakePath(PATH_ASCII, old_filename), archive, fsMakePath(PATH_ASCII, new_filename))))
		return ret;
	
	return 0;
}

Result fsOpen(Handle * handle, FS_Archive archive, const char * path, u32 flags)
{
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_OpenFile(handle, archive, fsMakePath(PATH_ASCII, path), flags, 0)))
		return ret;
	
	return 0;
}

Result fsWrite(FS_Archive archive, const char * path, const char * buf)
{
	Handle handle;
	Result ret = 0;
	
	u32 len = strlen(buf);
	u64 size = 0;
	u32 bytesWritten = 0;

	if (fileExists(archive, path))
		fsRemove(archive, path);

	if (R_FAILED(ret = fsOpen(&handle, archive, path, (FS_OPEN_WRITE | FS_OPEN_CREATE))))
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