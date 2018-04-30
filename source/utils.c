#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "fs.h"
#include "utils.h"

const char * configFile =
	"sortBy = %d\n"
	"recycleBin = %d\n"
	"systemProtection = %d\n"
	"showHiddenFiles = %d";

void Utils_MakeDirectories(void)
{
	if (BROWSE_STATE != STATE_NAND)
	{
		if (!(FS_DirExists(archive, "/3ds/3DShell/themes/default/")))
			FS_RecursiveMakeDir(archive, "/3ds/3DShell/themes/default");
		if (!(FS_DirExists(archive, "/3ds/3DShell/bin/")))
			FS_RecursiveMakeDir(archive, "/3ds/3DShell/bin");
	}
}

Result Utils_SaveConfig(int sortBy, bool recycleBin, bool protection, bool hidden)
{
	Result ret = 0;
	
	char * buf = (char *)malloc(256);
	snprintf(buf, 256, configFile, sortBy, recycleBin, protection, hidden);
	
	if (R_FAILED(ret = FS_Write(archive, "/3ds/3DShell/config.cfg", buf)))
	{
		free(buf);
		return ret;
	}
	
	free(buf);
	return 0;
}	
	
Result Utils_LoadConfig(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!FS_FileExists(archive, "/3ds/3DShell/config.cfg"))
	{
		// set these to the following by default:
		sortBy = 1;
		recycleBin = 0;
		galleryDisplay = 1;
		isHiddenEnabled = 0;		
		return Utils_SaveConfig(sortBy, recycleBin, galleryDisplay, isHiddenEnabled);
	}

	u64 size64 = 0;
	u32 size = 0;

	size64 = FS_GetFileSize(archive, "/3ds/3DShell/config.cfg");
	size = (u32)size64;
	char * buf = (char *)malloc(size + 1);

	if (R_FAILED(ret = FS_Read(archive, "/3ds/3DShell/config.cfg", size, buf)))
	{
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	
	sscanf(buf, configFile, &sortBy, &recycleBin, &galleryDisplay, &isHiddenEnabled);
	
	free(buf);
	return 0;
}

Result Utils_GetLastDirectory(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!FS_FileExists(archive, "/3ds/3DShell/lastdir.txt"))
	{
		FS_Write(archive, "/3ds/3DShell/lastdir.txt", START_PATH);
		strcpy(cwd, START_PATH); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	else
	{
		u64 size64 = 0;
		u32 size = 0;

		size64 = FS_GetFileSize(archive, "/3ds/3DShell/lastdir.txt");
		size = (u32)size64;
		char * buf = (char *)malloc(size + 1);

		if (R_FAILED(ret = FS_Read(archive, "/3ds/3DShell/lastdir.txt", size, buf)))
		{
			free(buf);
			return ret;
		}

		buf[size] = '\0';

		char tempPath[256];
		sscanf(buf, "%[^\n]s", tempPath);
	
		if (FS_DirExists(archive, tempPath)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, tempPath);
		else
			strcpy(cwd, START_PATH);
		
		free(buf);
	}
	
	return 0;
}

char * Utils_Basename(const char * filename)
{
	char *p = strrchr (filename, '/');
	return p ? p + 1 : (char *) filename;
}

void Utils_GetSizeString(char * string, uint64_t size) //Thanks TheOfficialFloW
{
	double double_size = (double)size;

	int i = 0;
	static char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };

	while (double_size >= 1024.0f)
	{
		double_size /= 1024.0f;
		i++;
	}

	sprintf(string, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

CFG_Region Utils_GetRegion(void)
{
	CFG_Region region = 0;

	if (R_SUCCEEDED(CFGU_SecureInfoGetRegion(&region)))
		return region;

	return 0;
}

CFG_Language Utils_GetLanguage(void)
{
	CFG_Language language = 0;

	if (R_SUCCEEDED(CFGU_GetSystemLanguage(&language)))
		return language;

	return 0;
}

const char * Utils_GetUsername(void)
{
	u8 data[0x1C];
	static wchar_t whcar_username[0x13];
	char * username = (char *)malloc(0x1C);

	if (R_SUCCEEDED(CFGU_GetConfigInfoBlk2(0x1C, 0x000A0000, data)))
	{
		for (int i = 0; i < 0x13; i++)
			whcar_username[i] = (wchar_t)((u16 *)data)[i];
	}
	else
	{
		free(username);
		return NULL;
	}

	wcstombs(username, whcar_username, 0x1C);

	return username;
}

bool Utils_IsN3DS(void)
{
	bool isNew3DS = false;

	if (R_SUCCEEDED(APT_CheckNew3DS(&isNew3DS)))
		return isNew3DS;

	return false;
}

void u16_to_u8(char * buf, const u16 * input, size_t bufsize)
{
	ssize_t units = utf16_to_utf8((u8 *)buf, input, bufsize);

	if (units < 0)
		units = 0;

	buf[units] = 0;
}