#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "utils.h"

FS_Archive archive, sdmc_archive, nand_archive;

Result FS_OpenArchive(FS_Archive *archive, FS_ArchiveID archiveID) {
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_OpenArchive(archive, archiveID, fsMakePath(PATH_EMPTY, ""))))
		return ret;

	return 0;
}

Result FS_CloseArchive(FS_Archive archive) {
	Result ret = 0;

	if (R_FAILED(ret = FSUSER_CloseArchive(archive)))
		return ret;

	return 0;
}

Result FS_OpenDir(Handle *handle, FS_Archive archive, const char *path) {
	Result ret = 0;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_OpenDirectory(handle, archive, fsMakePath(PATH_UTF16, path_u16))))
		return ret;
	
	return 0;
}

Result FS_OpenFile(Handle *handle, FS_Archive archive, const char *path, u32 flags, u32 attributes) {
	Result ret = 0;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_OpenFile(handle, archive, fsMakePath(PATH_UTF16, path_u16), flags, attributes)))
		return ret;
	
	return 0;
}

Result FS_MakeDir(FS_Archive archive, const char *path) {
	Result ret = 0;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path_u16), 0)))
		return ret;
	
	return 0;
}

Result FS_CreateFile(FS_Archive archive, const char *path) {
	Result ret = 0;
	
	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path_u16), 0, 0)))
		return ret;
	
	return 0;
}

Result FS_RecursiveMakeDir(FS_Archive archive, const char *path) {
	Result ret = 0;
	char buf[256];
	char *p = NULL;
	size_t len;

	snprintf(buf, sizeof(buf), "%s", path);
	len = strlen(buf);

	if (buf[len - 1] == '/')
		buf[len - 1] = 0;

	for (p = buf + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;

			if (!FS_DirExists(archive, buf))
				ret = FS_MakeDir(archive, buf);
			
			*p = '/';
		}
		
		if (!FS_DirExists(archive, buf))
			ret = FS_MakeDir(archive, buf);
	}
	
	return ret;
}

bool FS_FileExists(FS_Archive archive, const char *path) {
	Handle handle;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path_u16), FS_OPEN_READ, 0)))
		return false;

	if (R_FAILED(FSFILE_Close(handle)))
		return false;

	return true;
}

bool FS_DirExists(FS_Archive archive, const char *path) {
	Handle handle;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_UTF16, path_u16))))
		return false;

	if (R_FAILED(FSDIR_Close(handle)))
		return false;

	return true;
}

Result FS_GetFileSize(FS_Archive archive, const char *path, u64 *size) {
	Result ret = 0;
	Handle handle;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path_u16), FS_OPEN_READ, 0)))
		return ret;

	if (R_FAILED(ret = FSFILE_GetSize(handle, size))) {
		FSFILE_Close(handle);
		return ret;
	}

	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;
	
	return 0;
}

u64 FS_GetFreeStorage(FS_SystemMediaType media_type) {
	FS_ArchiveResource resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, media_type)))
		return (((u64)resource.freeClusters * (u64)resource.clusterSize));

	return 0;
}

u64 FS_GetTotalStorage(FS_SystemMediaType media_type) {
	FS_ArchiveResource resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, media_type)))
		return (((u64)resource.totalClusters * (u64)resource.clusterSize));

	return 0;
}

u64 FS_GetUsedStorage(FS_SystemMediaType media_type) {
	return (FS_GetTotalStorage(media_type) - FS_GetUsedStorage(media_type));
}

Result FS_RemoveFile(FS_Archive archive, const char *path) {
	Result ret = 0;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_DeleteFile(archive, fsMakePath(PATH_UTF16, path_u16))))
		return ret;
	
	return 0;
}

Result FS_RemoveDir(FS_Archive archive, const char *path) {
	Result ret = 0;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_DeleteDirectory(archive, fsMakePath(PATH_UTF16, path_u16))))
		return ret;
	
	return 0;
}

Result FS_RemoveDirRecursive(FS_Archive archive, const char *path) {
	Result ret = 0;

	u16 path_u16[strlen(path) + 1];
	Utils_U8_To_U16(path_u16, (const u8 *)path, strlen(path) + 1);

	if (R_FAILED(ret = FSUSER_DeleteDirectoryRecursively(archive, fsMakePath(PATH_UTF16, path_u16))))
		return ret;
	
	return 0;
}

Result FS_RenameFile(FS_Archive archive, const char *old_filename, const char *new_filename) {
	Result ret = 0;

	u16 old_filename_u16[strlen(old_filename) + 1];
	Utils_U8_To_U16(old_filename_u16, (const u8 *)old_filename, strlen(old_filename) + 1);

	u16 new_filename_u16[strlen(new_filename) + 1];
	Utils_U8_To_U16(new_filename_u16, (const u8 *)new_filename, strlen(new_filename) + 1);

	if (R_FAILED(ret = FSUSER_RenameFile(archive, fsMakePath(PATH_UTF16, old_filename_u16), archive, fsMakePath(PATH_UTF16, new_filename_u16))))
		return ret;
	
	return 0;
}

Result FS_RenameDir(FS_Archive archive, const char *old_dirname, const char *new_dirname) {
	Result ret = 0;

	u16 old_dirname_u16[strlen(old_dirname) + 1];
	Utils_U8_To_U16(old_dirname_u16, (const u8 *)old_dirname, strlen(old_dirname) + 1);

	u16 new_dirname_u16[strlen(new_dirname) + 1];
	Utils_U8_To_U16(new_dirname_u16, (const u8 *)new_dirname, strlen(new_dirname) + 1);

	if (R_FAILED(ret = FSUSER_RenameDirectory(archive, fsMakePath(PATH_UTF16, old_dirname_u16), archive, fsMakePath(PATH_UTF16, new_dirname_u16))))
		return ret;
	
	return 0;
}

Result FS_Read(FS_Archive archive, const char *path, u64 size, void *buf) {
	Result ret = 0;
	Handle handle;

	u32 bytes_read = 0;

	if (R_FAILED(ret = FS_OpenFile(&handle, archive, path, FS_OPEN_READ, 0)))
		return ret;
	
	if (R_FAILED(ret = FSFILE_Read(handle, &bytes_read, 0, buf, size))) {
		FSFILE_Close(handle);
		return ret;
	}

	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;

	return 0;
}

Result FS_Write(FS_Archive archive, const char *path, const void *buf) {
	Result ret = 0;
	Handle handle;
	
	u32 len = strlen(buf);
	u64 size = 0;
	u32 bytes_written = 0;

	if (FS_FileExists(archive, path))
		FS_RemoveFile(archive, path);

	if (R_FAILED(ret = FS_OpenFile(&handle, archive, path, (FS_OPEN_WRITE | FS_OPEN_CREATE), 0)))
		return ret;

	if (R_FAILED(ret = FSFILE_GetSize(handle, &size))) {
		FSFILE_Close(handle);
		return ret;
	}

	if (R_FAILED(ret = FSFILE_SetSize(handle, size + len))) {
		FSFILE_Close(handle);
		return ret;
	}

	if (R_FAILED(ret = FSFILE_Write(handle, &bytes_written, size, buf, len, FS_WRITE_FLUSH))) {
		FSFILE_Close(handle);
		return ret;
	}

	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;

	return 0;
}

char *FS_GetFileTimestamp(const char *path) {
	static char timeStr[60];
	u64 mtime = 0;

	if (R_SUCCEEDED(sdmc_getmtime(path, &mtime))) {
		time_t mt = mtime;
		struct tm *timeStruct = gmtime(&mt);

		int hours = timeStruct->tm_hour;
		int minutes = timeStruct->tm_min;

		int day = timeStruct->tm_mday;
		int month = timeStruct->tm_mon + 1; // January being 0
		int year = timeStruct->tm_year + 1900;

		snprintf(timeStr, 60, "%d/%d/%d %2i:%02i", year, month, day, hours, minutes);
	}
	else
		return NULL;

	return timeStr;
}
