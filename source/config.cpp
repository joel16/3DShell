#include <cstdio>
#include <cstring>
#include <jansson.h>
#include <string>

#include <3ds.h>
#include "config.h"
#include "fs.h"
#include "log.h"
#include "utils.h"

#define CONFIG_VERSION 2

ConfigData cfg;

namespace Config {
    constexpr const char16_t *path = u"/3ds/3DShell/config.json";
    static int configVersion = 0;
    
    static void SetDefault(ConfigData &config) {
        config.sort = 0;
        config.debug = false;
        config.theme = false;
        config.cwd = u"/";
    }
    
    int Save(const ConfigData &config) {
        Result ret = 0;
        std::string cwd_utf8 = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(config.cwd.c_str()));
        
        json_t *root = json_pack("{s:i, s:i, s:b, s:b, s:s}",
            "config_ver", CONFIG_VERSION,
            "sort", config.sort,
            "debug", config.debug,
            "theme", config.theme,
            "cwd", cwd_utf8.c_str()
        );

        if (!root) {
            Log::Error("Failed to create JSON object for config.");
            return -1;
        }

        char *json_str = json_dumps(root, JSON_INDENT(4));
        json_decref(root);

        if (!json_str) {
            Log::Error("Failed to encode config JSON.");
            return -1;
        }

        // Delete and recreate config file
        FSUSER_DeleteFile(sdmcArchive, fsMakePath(PATH_UTF16, path));
        FSUSER_CreateFile(sdmcArchive, fsMakePath(PATH_UTF16, path), 0, strlen(json_str));

        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, path), FS_OPEN_WRITE, 0))) {
            Log::Error("FSUSER_OpenFile(config.json) failed: 0x%x\n", ret);
            free(json_str);
            return ret;
        }

        u32 bytesWritten = 0;
        ret = FSFILE_Write(file, &bytesWritten, 0, json_str, strlen(json_str), FS_WRITE_FLUSH);
        FSFILE_Close(file);
        free(json_str);

        if (R_FAILED(ret) || bytesWritten == 0) {
            Log::Error("FSFILE_Write(config.json) failed: 0x%x\n", ret);
            return ret;
        }

        return 0;
    }
    
    int Load(void) {
        Result ret = 0;

        if (!FS::DirExists(sdmcArchive, u"/3ds/")) {
            FSUSER_CreateDirectory(sdmcArchive, fsMakePath(PATH_ASCII, "/3ds"), 0);
        }

        if (!FS::DirExists(sdmcArchive, u"/3ds/3DShell/")) {
            FSUSER_CreateDirectory(sdmcArchive, fsMakePath(PATH_ASCII, "/3ds/3DShell"), 0);
        }

        // Create default config if missing
        if (!FS::FileExists(sdmcArchive, path)) {
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }

        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, path), FS_OPEN_READ, 0))) {
            Log::Error("Failed to open config.json: 0x%x\n", ret);
            return ret;
        }

        // Read file contents
        u64 size = 0;
        if (R_FAILED(FSFILE_GetSize(file, &size)) || size == 0) {
            FSFILE_Close(file);
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }

        std::string json_str;
        json_str.resize(size);
        u32 bytesRead = 0;

        ret = FSFILE_Read(file, &bytesRead, 0, &json_str[0], size);
        FSFILE_Close(file);

        if (R_FAILED(ret) || bytesRead != size) {
            Log::Error("Failed to read config.json: 0x%x\n", ret);
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }

        // Parse JSON
        json_error_t error;
        json_t *root = json_loads(json_str.c_str(), JSON_DISABLE_EOF_CHECK, &error);

        if (!root) {
            Log::Error("JSON parse error at line %d: %s", error.line, error.text);
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }
        
        configVersion = json_integer_value(json_object_get(root, "config_ver"));
        cfg.sort = json_integer_value(json_object_get(root, "sort"));
        cfg.debug = json_is_true(json_object_get(root, "debug"));
        cfg.theme = json_is_true(json_object_get(root, "theme"));

        json_t *cwd = json_object_get(root, "cwd");
        if (json_is_string(cwd)) {
            cfg.cwd = Utils::UTF8ToUTF16(json_string_value(cwd));
        } else {
            cfg.cwd = u"/";
        }

        json_decref(root);
        
        if (!FS::DirExists(sdmcArchive, cfg.cwd)) {
            cfg.cwd = u"/";
        }
        
        if (configVersion < CONFIG_VERSION) {
            FSUSER_DeleteFile(sdmcArchive, fsMakePath(PATH_UTF16, path));
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }

        return 0;
    }
}
