#include <stdio.h>

#include "file/dirlist.h"
#include "file/fs.h"
#include "utils.h"

Result openArchive(FS_Archive * archive, FS_ArchiveID id)
{
	return FSUSER_OpenArchive(archive, id, fsMakePath(PATH_EMPTY, ""));
}

Result closeArchive(FS_Archive archive)
{
	return FSUSER_CloseArchive(archive);
}

Result makeDir(FS_Archive archive, const char * path)
{
	if ((!archive) || (!path))
		return -1;

	return FSUSER_CreateDirectory(archive, fsMakePath(PATH_ASCII, path), 0);
}

bool fileExists(FS_Archive archive, const char * path)
{
	if ((!path) || (!archive))
		return false;

	Handle handle;

	if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0)))
		return false;

	if (R_FAILED(FSFILE_Close(handle)))
		return false;

	return true;
}

bool dirExists(FS_Archive archive, const char * path)
{
	if ((!path) || (!archive))
		return false;

	Handle handle;

	if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_ASCII, path))))
		return false;

	if (R_FAILED(FSDIR_Close(handle)))
		return false;

	return true;
}

char * getFileModifiedTime(char * path)
{
	static char timeStr[30];
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

		switch(getRegion())
		{
			case CFG_REGION_JPN:
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", year, month, day, hours, minutes, amOrPm ? "AM" : "PM");
				break;
			case CFG_REGION_USA: // This is the worst one
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", month, day, year, hours, minutes, amOrPm ? "AM" : "PM");
				break;
			case CFG_REGION_EUR:
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", day, month, year, hours, minutes, amOrPm ? "AM" : "PM");
				break;
			case CFG_REGION_AUS:
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", day, month, year, hours, minutes, amOrPm ? "AM" : "PM");
				break;
			case CFG_REGION_CHN:
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", year, month, day, hours, minutes, amOrPm ? "AM" : "PM");
				break;
			case CFG_REGION_KOR:
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", year, month, day, hours, minutes, amOrPm ? "AM" : "PM");
				break;
			case CFG_REGION_TWN:
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", year, month, day, hours, minutes, amOrPm ? "AM" : "PM");
				break;
			default:
				sprintf(timeStr, "%d/%d/%d %2i:%02i %s", day, month, year, hours, minutes, amOrPm ? "AM" : "PM");
				break;
		}
	}

	return timeStr;
}

u64 getFileSize(FS_Archive archive, const char * path)
{
	u64 st_size = 0;
	Handle handle;

	if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0)))
		return 0;

	if (R_FAILED(FSFILE_GetSize(handle, &st_size)))
		return 0;

	if (R_FAILED(FSFILE_Close(handle)))
		return 0;

	return st_size;
}

Result fsRemove(FS_Archive archive, const char * filename)
{
	return FSUSER_DeleteFile(archive, fsMakePath(PATH_ASCII, filename));
}

Result fsRmdir(FS_Archive archive, const char * path)
{
	return FSUSER_DeleteDirectory(archive, fsMakePath(PATH_ASCII, path));
}

Result fsRmdirRecursive(FS_Archive archive, const char * path)
{
	return FSUSER_DeleteDirectoryRecursively(archive, fsMakePath(PATH_ASCII, path));
}

Result fsRenameFile(FS_Archive archive, const char * old_filename, const char * new_filename)
{
	return FSUSER_RenameFile(archive, fsMakePath(PATH_ASCII, old_filename), archive, fsMakePath(PATH_ASCII, new_filename));
}

Result fsRenameDir(FS_Archive archive, const char * old_filename, const char * new_filename)
{
	return FSUSER_RenameDirectory(archive, fsMakePath(PATH_ASCII, old_filename), archive, fsMakePath(PATH_ASCII, new_filename));
}

Result fsOpen(Handle * handle, const char * path, u32 flags)
{
	FS_ArchiveID id;

	if (BROWSE_STATE == STATE_SD)
		id = ARCHIVE_SDMC;
	else
		id = ARCHIVE_NAND_CTR_FS;

	return FSUSER_OpenFileDirectly(handle, id, fsMakePath(PATH_EMPTY, ""), fsMakePath(PATH_ASCII, path), flags, 0);
}

Result writeFile(const char * path, const char * buf)
{
	Handle handle;
	u32 len = strlen(buf);
	u64 size;
	u32 written;

	if (fileExists(fsArchive, path))
		fsRemove(fsArchive, path);

	Result ret = fsOpen(&handle, path, (FS_OPEN_WRITE | FS_OPEN_CREATE));
	if (R_FAILED(ret))
		return ret;

	ret = FSFILE_GetSize(handle, &size);
	if (R_FAILED(ret))
		return ret;

	ret = FSFILE_SetSize(handle, size + len);
	if (R_FAILED(ret))
		return ret;

	ret = FSFILE_Write(handle, &written, size, buf, len, FS_WRITE_FLUSH);
	if (R_FAILED(ret))
		return ret;

	ret = FSFILE_Close(handle);
	if (R_FAILED(ret))
		return ret;

	return 0;
}
