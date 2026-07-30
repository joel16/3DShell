#pragma once

#include <string>

#include "reader.h"

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
    int UpdateBookEntry(const char *path, int page, float zoom);
    int GetBookEntry(const char *path, BookEntry &entry);
}
