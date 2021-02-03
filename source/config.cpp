#include <cstdio>
#include <jansson.h>
#include <string>

#include "config.h"
#include "fs.h"
#include "log.h"

#define CONFIG_VERSION 1

config_t cfg;

namespace Config {
    static const char *config_file = "{\n\t\"config_ver\": %d,\n\t\"sort\": %d,\n\t\"dev_options\": %d,\n\t\"dark_theme\": %d,\n\t\"last_dir\": \"%s\"\n}";
    static int config_version_holder = 0;
    static std::string config_path = "/3ds/3DShell/config.json";
    
    int Save(config_t config) {
        Result ret = 0;
        char *buf = new char[1024];
        u32 length = std::snprintf(buf, 1024, config_file, CONFIG_VERSION, config.sort, config.dev_options, config.dark_theme, config.cwd.c_str());
        
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
        root = json_loads(buf, 0, &error);
        delete[] buf;
        
        if (!root)
            return -1;
        
        json_t *config_ver = json_object_get(root, "config_ver");
        config_version_holder = json_integer_value(config_ver);
        
        json_t *sort = json_object_get(root, "sort");
        cfg.sort = json_integer_value(sort);
        
        json_t *dev_options = json_object_get(root, "dev_options");
        cfg.dev_options = json_integer_value(dev_options);

        json_t *dark_theme = json_object_get(root, "dark_theme");
        cfg.dark_theme = json_integer_value(dark_theme);
        
        json_t *last_dir = json_object_get(root, "last_dir");
        cfg.cwd = json_string_value(last_dir);
        
        if (!FS::DirExists(sdmc_archive, cfg.cwd))
            cfg.cwd = "/";
            
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
