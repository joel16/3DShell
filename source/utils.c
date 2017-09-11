#include "common.h"
#include "file/fs.h"
#include "utils.h"

const char * configFile = 
	"lastdir=%s\n"
	"bgm=%d\n"
	"systemProtection=%d\n"
	"showHiddenFiles=%d\n"
	"showHiddenFiles=%d\n";

void loadConfig(void)
{
	Handle handle;
	
	if (!fileExists(fsArchive, "/3ds/data/3DShell/config.cfg"))
	{
		if (R_FAILED(fsOpen(&handle, "/3ds/data/3DShell/config.cfg", FS_OPEN_CREATE | FS_OPEN_WRITE)))
			return;
	
		// set these to the following by default:
		bgmEnable = 0;
		sysProtection = 1;
		isHiddenEnabled = 1;
		
		char * tempbuf = (char *)malloc(1024);
		u32 size = snprintf(tempbuf, 1024, configFile, START_PATH, bgmEnable, sysProtection, isHiddenEnabled);
		
		FSFILE_SetSize(handle, (u64)size);
		
		u32 byteswritten = 0;
		FSFILE_Write(handle, &byteswritten, 0, (u32 *)tempbuf, size, FS_WRITE_FLUSH);
	
		FSFILE_Close(handle);
		free(tempbuf);
	}
	
	if (R_FAILED(fsOpen(&handle, "/3ds/data/3DShell/config.cfg", FS_OPEN_READ)))
		return;
	
	u64 size64 = 0;
	u32 size = 0;
	
	if (R_SUCCEEDED(FSFILE_GetSize(handle, &size64)))
		size = (u32)size64;
	
	char * tempbuf = (char *)malloc(size + 1);
	u32 bytesread = 0;
	FSFILE_Read(handle, &bytesread, 0, (u32 *)tempbuf, size);
	tempbuf[size] = '\0';
	
	char tempPath[250];
	
	sscanf(tempbuf, configFile, tempPath, &bgmEnable, &sysProtection, &isHiddenEnabled);
	
	if (dirExists(fsArchive, tempPath)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
		strcpy(cwd, tempPath);
	else
		strcpy(cwd, START_PATH);
	
	FSFILE_Close(handle);
	free(tempbuf);
}

void makeDirectories(void)
{
	if (BROWSE_STATE != STATE_NAND)
	{
		if (!(dirExists(fsArchive, "/3ds/")))
			makeDir(fsArchive, "/3ds");
		if (!(dirExists(fsArchive, "/3ds/data/")))
			makeDir(fsArchive, "/3ds/data");
		if (!(dirExists(fsArchive, "/3ds/data/3DShell/")))
			makeDir(fsArchive, "/3ds/data/3DShell");
		if (!(dirExists(fsArchive, "/3ds/data/3DShell/themes/")))
			makeDir(fsArchive, "/3ds/data/3DShell/themes");
		if (!(dirExists(fsArchive, "/3ds/data/3DShell/themes/default/")))
			makeDir(fsArchive, "/3ds/data/3DShell/themes/default");
		if (!(dirExists(fsArchive, "/3ds/data/3DShell/colours/")))
			makeDir(fsArchive, "/3ds/data/3DShell/colours");
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

Result setConfig(const char * path, bool set) // using individual txt files for configs for now (plan to change this later when there's more options)
{
	if (set)
		return writeFile(path, "1");

	return writeFile(path, "0");
}

const char * getLastNChars(char * str, int n)
{
	int len = strlen(str);
	
	const char * last_n = &str[len - n];
	
	return last_n;
}

u8 getRegion(void)
{
	u8 region = 0;

	if (R_SUCCEEDED(CFGU_SecureInfoGetRegion(&region)))
		return region;

	return 0;
}

u8 getLanguage(void)
{
	u8 language = 0;

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