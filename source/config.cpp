#include <cstdio>
#include <cstring>
#include <jansson.h>
#include <string>

#include "config.h"
#include "fs.h"
#include "log.h"
#include "utils.h"

#define CONFIG_VERSION 2

ConfigData cfg;

namespace Config {
    constexpr const char16_t *configPath = u"/3ds/3DShell/config.json";
    constexpr const char16_t *booksConfigPath = u"/3ds/3DShell/books.json";
    
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

        char *json = json_dumps(root, JSON_INDENT(4));
        json_decref(root);

        if (!json) {
            Log::Error("Failed to encode config JSON.");
            return -1;
        }

        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, configPath), FS_OPEN_WRITE | FS_OPEN_CREATE, 0))) {
            Log::Error("FSUSER_OpenFile(config.json) failed: 0x%x\n", ret);
            free(json);
            return ret;
        }

        // Truncate the file to match the new JSON length instead of deleting and re-creating the config.
        FSFILE_SetSize(file, std::strlen(json));

        u32 bytesWritten = 0;
        ret = FSFILE_Write(file, &bytesWritten, 0, json, std::strlen(json), FS_WRITE_FLUSH);
        FSFILE_Close(file);
        free(json);

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
        if (!FS::FileExists(sdmcArchive, configPath)) {
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }

        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, configPath), FS_OPEN_READ, 0))) {
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

        std::string json;
        json.resize(size);
        u32 bytesRead = 0;

        ret = FSFILE_Read(file, &bytesRead, 0, &json[0], size);
        FSFILE_Close(file);

        if (R_FAILED(ret) || bytesRead != size) {
            Log::Error("Failed to read config.json: 0x%x\n", ret);
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }

        // Parse JSON
        json_error_t error;
        json_t *root = json_loads(json.c_str(), JSON_DISABLE_EOF_CHECK, &error);

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
        }
        else {
            cfg.cwd = u"/";
        }

        json_decref(root);
        
        if (!FS::DirExists(sdmcArchive, cfg.cwd)) {
            cfg.cwd = u"/";
        }
        
        if (configVersion < CONFIG_VERSION) {
            FSUSER_DeleteFile(sdmcArchive, fsMakePath(PATH_UTF16, configPath));
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }

        return 0;
    }

    int UpdateBookEntry(const char *path, int page, float zoom) {
        Result ret = 0;
        std::string utf8Path = path ? path : "";

        if (!FS::DirExists(sdmcArchive, u"/3ds/")) {
            FSUSER_CreateDirectory(sdmcArchive, fsMakePath(PATH_ASCII, "/3ds"), 0);
        }
        if (!FS::DirExists(sdmcArchive, u"/3ds/3DShell/")) {
            FSUSER_CreateDirectory(sdmcArchive, fsMakePath(PATH_ASCII, "/3ds/3DShell"), 0);
        }

        json_t *root = nullptr;
        json_error_t error;

        if (FS::FileExists(sdmcArchive, booksConfigPath)) {
            Handle file;
            if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, booksConfigPath), FS_OPEN_READ, 0))) {
                Log::Error("Failed to open books.json for reading: 0x%x\n", ret);
                return ret;
            }

            u64 size = 0;
            if (R_FAILED(FSFILE_GetSize(file, &size)) || size == 0) {
                FSFILE_Close(file);
                root = json_array();
            }
            else {
                std::string json;
                json.resize(size);
                u32 bytesRead = 0;
                FSFILE_Read(file, &bytesRead, 0, &json[0], size);
                FSFILE_Close(file);

                root = json_loads(json.c_str(), JSON_DISABLE_EOF_CHECK, &error);
                if (!root || !json_is_array(root)) {
                    if (root) {
                        json_decref(root);
                    }
                    root = json_array();
                }
            }
        }
        else {
            root = json_array();
        }

        bool found = false;
        size_t index = 0;
        json_t *bookObj = nullptr;

        json_array_foreach(root, index, bookObj) {
            json_t *pathJson = json_object_get(bookObj, "path");
            if (json_is_string(pathJson) && utf8Path == json_string_value(pathJson)) {
                json_object_set_new(bookObj, "page", json_integer(page));
                json_object_set_new(bookObj, "zoom", json_real(zoom));
                found = true;
                break;
            }
        }

        if (!found) {
            json_t *newBook = json_pack("{s:s, s:i, s:f}", 
                "path", utf8Path.c_str(), 
                "page", page, 
                "zoom", zoom
            );
            json_array_append_new(root, newBook);
        }

        char *json = json_dumps(root, JSON_INDENT(4));
        json_decref(root);

        if (!json) {
            Log::Error("Failed to encode books JSON.");
            return -1;
        }

        FSUSER_DeleteFile(sdmcArchive, fsMakePath(PATH_UTF16, booksConfigPath));
        FSUSER_CreateFile(sdmcArchive, fsMakePath(PATH_UTF16, booksConfigPath), 0, strlen(json));

        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, booksConfigPath), FS_OPEN_WRITE, 0))) {
            Log::Error("FSUSER_OpenFile(books.json) failed: 0x%x\n", ret);
            free(json);
            return ret;
        }

        u32 bytesWritten = 0;
        ret = FSFILE_Write(file, &bytesWritten, 0, json, strlen(json), FS_WRITE_FLUSH);
        FSFILE_Close(file);
        free(json);

        if (R_FAILED(ret) || bytesWritten == 0) {
            Log::Error("FSFILE_Write(books.json) failed: 0x%x\n", ret);
            return ret;
        }

        return 0;
    }

    int GetBookEntry(const char *path, BookEntry &outEntry) {
        Result ret = 0;
        std::string utf8Path = path ? path : "";

        if (!FS::FileExists(sdmcArchive, booksConfigPath)) {
            return -1;
        }

        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, booksConfigPath), FS_OPEN_READ, 0))) {
            Log::Error("Failed to open books.json for reading: 0x%x\n", ret);
            return ret;
        }

        u64 size = 0;
        if (R_FAILED(FSFILE_GetSize(file, &size)) || size == 0) {
            FSFILE_Close(file);
            return -1;
        }

        std::string json;
        json.resize(size);
        u32 bytesRead = 0;
        ret = FSFILE_Read(file, &bytesRead, 0, &json[0], size);
        FSFILE_Close(file);

        if (R_FAILED(ret) || bytesRead != size) {
            Log::Error("Failed to read books.json: 0x%x\n", ret);
            return ret;
        }

        json_error_t error;
        json_t *root = json_loads(json.c_str(), JSON_DISABLE_EOF_CHECK, &error);

        if (!root || !json_is_array(root)) {
            if (root) json_decref(root);
            return -1;
        }

        bool found = false;
        size_t index = 0;
        json_t *bookObj = nullptr;

        json_array_foreach(root, index, bookObj) {
            json_t *pathJson = json_object_get(bookObj, "path");
            json_t *pageJson = json_object_get(bookObj, "page");
            json_t *zoomJson = json_object_get(bookObj, "zoom");

            if (json_is_string(pathJson) && utf8Path == json_string_value(pathJson)) {
                outEntry.page = json_is_integer(pageJson) ? static_cast<int>(json_integer_value(pageJson)) : 0;
                outEntry.zoom = json_is_real(zoomJson) ? static_cast<float>(json_real_value(zoomJson)) : (json_is_integer(zoomJson) ? static_cast<float>(json_integer_value(zoomJson)) : 1.0f);
                found = true;
                break;
            }
        }
        
        json_decref(root);
        return found ? 0 : -1;
    }
}
