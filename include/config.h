#pragma once

#include <string>

typedef struct {
    int sort = 0;
    bool debug = true;
    bool theme = true;
    std::u16string cwd = u"/";
} ConfigData;

extern ConfigData cfg;

namespace Config {
    int Save(const ConfigData &config);
    int Load(void);
}
