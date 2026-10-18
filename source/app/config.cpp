#include <cstdio>
#include <algorithm>
#include <jansson.h>
#include <string>

#include "config.h"
#include "fs.h"
#include "log.h"

#define CONFIG_VERSION 1

config_t cfg;

namespace Config {
    static const char *config_file = "{\n\t\"config_ver\": %d,\n\t\"sort\": %d,\n\t\"dev_options\": %d,\n\t\"dark_theme\": %d,\n\t\"last_dir\": \"%s\",\n\t\"last_selected\": %d,\n\t\"last_scroll\": %d,\n\t\"filter\": %d\n}";
    static int config_version_holder = 0;
    static std::string config_path = "/3ds/3DShell/config.json";
    
    int Save(config_t config) {
        Result ret = 0;
        char *buf = new char[1024];
        u32 length = std::snprintf(buf, 1024, config_file, CONFIG_VERSION, config.sort, config.dev_options, config.dark_theme, config.cwd.c_str(), config.last_selected, config.last_scroll, config.filter);
        
        // Delete and re-create the file, we don't care about the return value here.
        FSUSER_DeleteFile(sdmc_archive, fsMakePath(PATH_ASCII, config_path.c_str()));
        FSUSER_CreateFile(sdmc_archive, fsMakePath(PATH_ASCII, config_path.c_str()), 0, length);
        
        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmc_archive, fsMakePath(PATH_ASCII, config_path.c_str()), FS_OPEN_WRITE, 0))) {
            Log::Error("FSUSER_OpenFile(/3ds/3DShell/config.json) failed: 0x%x\n", ret);
            delete[] buf;
            return ret;
        }
        
        u32 bytes_written = 0;
        if (R_FAILED(ret = FSFILE_Write(file, &bytes_written, 0, buf, length, FS_WRITE_FLUSH))) {
            Log::Error("FSFILE_Write(/3ds/3DShell/config.json) failed: 0x%x\n", ret);
            FSFILE_Close(file);
            delete[] buf;
            return ret;
        }

        if (bytes_written != length) {
            FSFILE_Close(file);
            delete[] buf;
            return ret;
        }
        
        FSFILE_Close(file);
        delete[] buf;
        return 0;
    }
    
    static void SetDefault(config_t *config) {
        config->sort = 0;
        config->dev_options = false;
        config->dark_theme = false;
        config->cwd = "/";
        config->filter = -1;
    }
    
    int Load(void) {
        Result ret = 0;
        
        if (!FS::DirExists(sdmc_archive, "/3ds/"))
            FSUSER_CreateDirectory(sdmc_archive, fsMakePath(PATH_ASCII, "/3ds"), 0);
        if (!FS::DirExists(sdmc_archive, "/3ds/3DShell/"))
            FSUSER_CreateDirectory(sdmc_archive, fsMakePath(PATH_ASCII, "/3ds/3DShell"), 0);
            
        if (!FS::FileExists(sdmc_archive, config_path.c_str())) {
            Config::SetDefault(&cfg);
            return Config::Save(cfg);
        }
        
        Handle file;
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmc_archive, fsMakePath(PATH_ASCII, config_path.c_str()), FS_OPEN_READ, 0)))
            return ret;
        
        u64 size = 0;
        if (R_FAILED(ret = FSFILE_GetSize(file, &size))) {
            FSFILE_Close(file);
            return ret;
        }

        char *buf =  new char[size + 1];
        u32 bytes_read = 0;

        if (R_FAILED(ret = FSFILE_Read(file, &bytes_read, 0, buf, size))) {
            FSFILE_Close(file);
            delete[] buf;
            return ret;
        }

        if (bytes_read != size) {
            FSFILE_Close(file);
            delete[] buf;
            return ret;
        }
        
        FSFILE_Close(file);

        json_t *root;
        json_error_t error;
        root = json_loads(buf, JSON_DISABLE_EOF_CHECK, &error);
        delete[] buf;
        
        if (!root) {
            Log::Error("Failed to decode config.json!\n");
            Config::SetDefault(&cfg);
            return Config::Save(cfg);
        }
        
        json_t *config_ver = json_object_get(root, "config_ver");
        config_version_holder = json_integer_value(config_ver);
        
        json_t *sort = json_object_get(root, "sort");
        cfg.sort = json_integer_value(sort);
        
        json_t *dev_options = json_object_get(root, "dev_options");
        cfg.dev_options = json_integer_value(dev_options);

        json_t *dark_theme = json_object_get(root, "dark_theme");
        cfg.dark_theme = json_integer_value(dark_theme);
        
        json_t *last_dir = json_object_get(root, "last_dir");
        const char *last_dir_str = json_string_value(last_dir);
        cfg.cwd = last_dir_str ? last_dir_str : "/";

        json_t *last_selected = json_object_get(root, "last_selected");
        json_int_t selected = json_integer_value(last_selected);
        if (selected < 0)
            selected = 0;
        cfg.last_selected = static_cast<int>(selected);
        
        json_t *last_scroll = json_object_get(root, "last_scroll");
        json_int_t scroll = json_integer_value(last_scroll);
        if (scroll < 0)
            scroll = 0;
        cfg.last_scroll = static_cast<int>(scroll);

        json_t *filter = json_object_get(root, "filter");
        /* Missing key (older configs) -> show all files. */
        if (json_is_integer(filter)) {
            json_int_t filter_value = json_integer_value(filter);
            if (filter_value < -1)
                filter_value = -1;
            cfg.filter = static_cast<int>(filter_value);
        }
        else
            cfg.filter = -1;

        if (!FS::DirExists(sdmc_archive, cfg.cwd)) {
            cfg.cwd = "/";
            cfg.last_selected = 0;
            cfg.last_scroll = 0;
        }
            
        // Delete config file if config file is updated. This will rarely happen.
        if (config_version_holder < CONFIG_VERSION) {
            FSUSER_DeleteFile(sdmc_archive, fsMakePath(PATH_ASCII, config_path.c_str()));
            Config::SetDefault(&cfg);
            return Config::Save(cfg);
        }
        
        json_decref(root);
        return 0;
    }
}
