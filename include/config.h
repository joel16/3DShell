#ifndef _3D_SHELL_CONFIG_H
#define _3D_SHELL_CONFIG_H

#include <3ds.h>

bool config_dark_theme, config_hidden_files;
int config_sort_by;

Result Config_Save(bool config_dark_theme, bool config_hidden_files, int config_sort_by);
Result Config_Load(void);
Result Config_GetLastDirectory(void);

#endif
