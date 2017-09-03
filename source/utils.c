#include "common.h"
#include "file/fs.h"
#include "utils.h"

void installDirectories(void)
{
	if (BROWSE_STATE != STATE_NAND)
	{
		if (!(dirExists(fsArchive, "/3ds/")))
			makeDir(fsArchive, "/3ds");
		if (!(dirExists(fsArchive, "/3ds/3DShell/")))
			makeDir(fsArchive, "/3ds/3DShell");
		if (!(dirExists(fsArchive, "/3ds/3DShell/themes/")))
			makeDir(fsArchive, "/3ds/3DShell/themes");
		if (!(dirExists(fsArchive, "/3ds/3DShell/themes/default/")))
			makeDir(fsArchive, "/3ds/3DShell/themes/default");
		if (!(dirExists(fsArchive, "/3ds/3DShell/colours/")))
			makeDir(fsArchive, "/3ds/3DShell/colours");

		if (fileExists(fsArchive, "/3ds/3DShell/lastdir.txt"))
		{
			char buf[250];

			FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
			fscanf(read, "%s", buf);
			fclose(read);

			if (dirExists(fsArchive, buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
				strcpy(cwd, buf);
			else
				strcpy(cwd, START_PATH);
		}
		else
		{
			writeFile("/3ds/3DShell/lastdir.txt", START_PATH);
			strcpy(cwd, START_PATH); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
		}

		/*if (!fileExists(fsArchive, "/3ds/3DShell/bgm.txt"))
		{
			setBgm(true);
		}
		else
		{
			int initBgm = 0;

			FILE * read = fopen("/3ds/3DShell/bgm.txt", "r");
			fscanf(read, "%d", &initBgm);
			fclose(read);

			if (initBgm == 0)
				bgmEnable = false;
			else
				bgmEnable = true;
		}*/

		if (!fileExists(fsArchive, "/3ds/3DShell/sysProtection.txt")) // Initially set it to true
		{
			setConfig("/3ds/3DShell/sysProtection.txt", true);
			sysProtection = true;
		}
		else
		{
			int info = 0;

			FILE * read = fopen("/3ds/3DShell/sysProtection.txt", "r");
			fscanf(read, "%d", &info);
			fclose(read);

			if (info)
				sysProtection = true;
			else
				sysProtection = false;
		}

		if (!fileExists(fsArchive, "/3ds/3DShell/isHidden.txt")) // Initially set it to true
		{
			setConfig("/3ds/3DShell/isHidden.txt", true);
			isHiddenEnabled = true;
		}
		else
		{
			int info = 0;

			FILE * read = fopen("/3ds/3DShell/isHidden.txt", "r");
			fscanf(read, "%d", &info);
			fclose(read);

			if (info)
				isHiddenEnabled = true;
			else
				isHiddenEnabled = false;
		}
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
	const char *last_n = &str[len - n];
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
	static wchar_t whcar_username[0x1C];
	char * username = (char *)malloc(0x1C);

	if (R_SUCCEEDED(CFGU_GetConfigInfoBlk2(0x1C, 0x000A0000, data)))
	{
		for (int i = 0; i < 0x1C; i++)
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
