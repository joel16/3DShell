#ifndef _3D_SHELL_CONFIG_H
#define _3D_SHELL_CONFIG_H

#include <3ds.h>
#include <string>

typedef struct {
	int sort = 0;
	bool dev_options = false;
	bool dark_theme = false;
	std::string cwd;
} config_t;

extern config_t cfg;

namespace Config {
	int Save(config_t config);
	int Load(void);
}

#endif
