#ifndef _3D_SHELL_CONFIG_H
#define _3D_SHELL_CONFIG_H

#include <3ds.h>

typedef struct {
	bool dark_theme;
	bool hidden_files;
	int sort;
} config_t;

extern config_t config;

Result Config_Save(config_t config);
Result Config_Load(void);
Result Config_GetLastDirectory(void);

#endif
