#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "fs.h"

#define CONFIG_VERSION 0

config_t config;
static int config_version_holder = 0;

const char *config_file =
	"config_ver = %d\n"
	"theme = %d\n"
	"hidden_files = %d\n"
	"sort = %d";

Result Config_Save(config_t config) {
	Result ret = 0;
	
	char *buf = malloc(64);
	snprintf(buf, 64, config_file, CONFIG_VERSION, config.dark_theme, config.hidden_files, config.sort);
	
	if (R_FAILED(ret = FS_Write(archive, "/3ds/3DShell/config.cfg", buf))) {
		free(buf);
		return ret;
	}
	
	free(buf);
	return 0;
}	
	
Result Config_Load(void) {
	Result ret = 0;
	
	if (!FS_FileExists(archive, "/3ds/3DShell/config.cfg")) {
		// set these to the following by default:
		config.dark_theme = false;
		config.hidden_files = false;
		config.sort = 0;
		return Config_Save(config);
	}

	u64 size = 0;
	FS_GetFileSize(archive, "/3ds/3DShell/config.cfg", &size);
	char *buf = malloc(size + 1);

	if (R_FAILED(ret = FS_Read(archive, "/3ds/3DShell/config.cfg", size, buf))) {
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	sscanf(buf, config_file, &config_version_holder, &config.dark_theme, &config.hidden_files, &config.sort);
	free(buf);

	// Delete config file if config file is updated. This will rarely happen.
	if (config_version_holder  < CONFIG_VERSION) {
		FS_RemoveFile(archive, "/3ds/3DShell/config.cfg");
		config.dark_theme = false;
		config.hidden_files = false;
		config.sort = 0;
		return Config_Save(config);
	}

	return 0;
}

Result Config_GetLastDirectory(void) {
	Result ret = 0;
	
	if (!FS_FileExists(archive, "/3ds/3DShell/lastdir.txt")) {
		FS_Write(archive, "/3ds/3DShell/lastdir.txt", START_PATH);
		strcpy(cwd, START_PATH); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	else {
		u64 size = 0;

		FS_GetFileSize(archive, "/3ds/3DShell/lastdir.txt", &size);
		char *buf = malloc(size + 1);

		if (R_FAILED(ret = FS_Read(archive, "/3ds/3DShell/lastdir.txt", size, buf))) {
			free(buf);
			return ret;
		}

		buf[size] = '\0';
		char path[513];
		sscanf(buf, "%[^\n]s", path);
	
		if (FS_DirExists(archive, path)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, path);
		else
			strcpy(cwd, START_PATH);
		
		free(buf);
	}
	
	return 0;
}
