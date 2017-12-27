#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "file/fs.h"
#include "utils.h"

const char * configFile =
	"recycleBin = %d\n"
	"systemProtection = %d\n"
	"showHiddenFiles = %d";

Result saveConfig(bool recycleBin, bool protection, bool hidden)
{
	Result ret = 0;
	
	char * buf = (char *)malloc(1024);
	snprintf(buf, 1024, configFile, recycleBin, protection, hidden);
	
	if (R_FAILED(ret = fsWrite(fsArchive, "/3ds/3DShell/config.cfg", buf)))
		return ret;
	
	free(buf);
	return 0;
}	
	
Result loadConfig(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!fileExists(fsArchive, "/3ds/3DShell/config.cfg"))
	{
		// set these to the following by default:
		recycleBin = 0;
		sysProtection = 1;
		isHiddenEnabled = 0;
		
		return saveConfig(recycleBin, sysProtection, isHiddenEnabled);
	}
	
	if (R_FAILED(ret = fsOpen(&handle, fsArchive, "/3ds/3DShell/config.cfg", FS_OPEN_READ)))
		return ret;
	
	u64 size64 = 0;
	u32 size = 0;
	
	if (R_FAILED(ret = FSFILE_GetSize(handle, &size64)))
		return ret;
		
	size = (u32)size64;
	
	char * buf = (char *)malloc(size + 1);
	u32 bytesread = 0;
	
	if (R_FAILED(ret = FSFILE_Read(handle, &bytesread, 0, (u32 *)buf, size)))
		return ret;
	
	buf[size] = '\0';
	
	sscanf(buf, configFile, &recycleBin, &sysProtection, &isHiddenEnabled);
	
	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;
	
	free(buf);
	return 0;
}

Result getLastDirectory(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!fileExists(fsArchive, "/3ds/3DShell/lastdir.txt"))
	{
		fsWrite(fsArchive, "/3ds/3DShell/lastdir.txt", START_PATH);
		strcpy(cwd, START_PATH); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	else
	{
		if (R_FAILED(ret = fsOpen(&handle, fsArchive, "/3ds/3DShell/lastdir.txt", FS_OPEN_READ)))
			return ret;
	
		u64 size64 = 0;
		u32 size = 0;
	
		if (R_FAILED(ret = FSFILE_GetSize(handle, &size64)))
			return ret;
		
		size = (u32)size64;
	
		char * buf = (char *)malloc(size + 1);
		u32 bytesread = 0;
		
		if (R_FAILED(ret = FSFILE_Read(handle, &bytesread, 0, (u32 *)buf, size)))
			return ret;
		
		buf[size] = '\0';
	
		char tempPath[250];
		sscanf(buf, "%s", tempPath);
	
		if (dirExists(fsArchive, tempPath)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, tempPath);
		else
			strcpy(cwd, START_PATH);
	
		if (R_FAILED(ret = FSFILE_Close(handle)))
			return ret;
		
		free(buf);
	}
	
	return 0;
}

char * basename(const char * filename)
{
	char *p = strrchr (filename, '/');
	return p ? p + 1 : (char *) filename;
}

void makeDirectories(void)
{
	if (BROWSE_STATE != STATE_NAND)
	{
		if (!(dirExists(fsArchive, "/3ds/3DShell/themes/default/")))
			recursiveMakeDir(fsArchive, "/3ds/3DShell/themes/default");
		if (!(dirExists(fsArchive, "/3ds/3DShell/bin/")))
			recursiveMakeDir(fsArchive, "/3ds/3DShell/bin");
	}
}

u64 getFreeStorage(FS_SystemMediaType mediaType)
{
	FS_ArchiveResource	resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, mediaType)))
		return (((u64) resource.freeClusters * (u64) resource.clusterSize));

	return 0;
}

u64 getTotalStorage(FS_SystemMediaType mediaType)
{
	FS_ArchiveResource	resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, mediaType)))
		return (((u64) resource.totalClusters * (u64) resource.clusterSize));

	return 0;
}

u64 getUsedStorage(FS_SystemMediaType mediaType)
{
	FS_ArchiveResource	resource = {0};

	if (R_SUCCEEDED(FSUSER_GetArchiveResource(&resource, mediaType)))
		return ((((u64) resource.totalClusters * (u64) resource.clusterSize)) - (((u64) resource.freeClusters * (u64) resource.clusterSize)));

	return 0;
}

void getSizeString(char * string, uint64_t size) //Thanks TheOfficialFloW
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

u16 touchGetX(void)
{
	touchPosition pos;
	hidTouchRead(&pos);
	
	return pos.px;
}

u16 touchGetY(void)
{
	touchPosition pos;
	hidTouchRead(&pos);
	
	return pos.py;
}

const char * getLastNChars(char * str, int n)
{
	int len = strlen(str);
	
	const char * last_n = &str[len - n];
	
	return last_n;
}

CFG_Region getRegion(void)
{
	CFG_Region region = 0;

	if (R_SUCCEEDED(CFGU_SecureInfoGetRegion(&region)))
		return region;

	return 0;
}

CFG_Language getLanguage(void)
{
	CFG_Language language = 0;

	if (R_SUCCEEDED(CFGU_GetSystemLanguage(&language)))
		return language;

	return 0;
}

const char * getUsername(void)
{
	u8 data[0x1C];
	static wchar_t whcar_username[0x13];
	char * username = (char *)malloc(0x1C);

	if (R_SUCCEEDED(CFGU_GetConfigInfoBlk2(0x1C, 0x000A0000, data)))
	{
		for (int i = 0; i < 0x13; i++)
			whcar_username[i] = (wchar_t)((u16 *)data)[i];
	}

	wcstombs(username, whcar_username, 0x1C);

	return username;
}

bool isN3DS(void)
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