#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "fs.h"

const char *config_file =
	"config_dark_theme = %d\n"
	"config_hidden_files = %d\n"
	"config_sort_by = %d";

Result Config_Save(bool config_dark_theme, bool config_hidden_files, int config_sort_by)
{
	Result ret = 0;
	
	char *buf = (char *)malloc(256);
	snprintf(buf, 256, config_file, config_dark_theme, config_hidden_files, config_sort_by);
	
	if (R_FAILED(ret = FS_Write(archive, "/3ds/3DShell/config.cfg", buf)))
	{
		free(buf);
		return ret;
	}
	
	free(buf);
	return 0;
}	
	
Result Config_Load(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!FS_FileExists(archive, "/3ds/3DShell/config.cfg"))
	{
		// set these to the following by default:
		config_dark_theme = false;
		config_hidden_files = true;
		config_sort_by = 0;
		return Config_Save(config_dark_theme, config_hidden_files, config_sort_by);
	}

	u64 size64 = 0;
	u32 size = 0;

	FS_GetFileSize(archive, "/3ds/3DShell/config.cfg", &size64);
	size = (u32)size64;
	char *buf = (char *)malloc(size + 1);

	if (R_FAILED(ret = FS_Read(archive, "/3ds/3DShell/config.cfg", size, buf)))
	{
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	
	sscanf(buf, config_file, &config_dark_theme, &config_hidden_files, &config_sort_by);
	
	free(buf);
	return 0;
}

Result Config_GetLastDirectory(void)
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

		FS_GetFileSize(archive, "/3ds/3DShell/lastdir.txt", &size64);
		size = (u32)size64;
		char *buf = (char *)malloc(size + 1);

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