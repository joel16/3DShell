#include <3ds.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <jansson.h>

#include "fs.h"
#include "log.h"
#include "net.h"

namespace Net {
    static s64 offset = 0;
    static void *soc_buf = nullptr;

    Result Init(void) {
        Result ret = 0;
        soc_buf = aligned_alloc(0x1000, 0x100000);

        if (!soc_buf)
            return -1;
            
        if (R_FAILED(ret = socInit(static_cast<u32* >(soc_buf), 0x100000))) {
            Log::Error("socInit() failed: 0x%x\n", ret);
            std::free(soc_buf);
            return ret;
        }

        return 0;
    }

    void Exit(void) {
        socExit();

        if (soc_buf)
            std::free(soc_buf);
    }
    
    bool GetNetworkStatus(void) {
        Result ret = 0;
        u32 status = 0;

        if (R_FAILED(ret = ACU_GetWifiStatus(&status))) {
            Log::Error("ACU_GetWifiStatus() failed: 0x%x\n", ret);
            return false;
        }

        return ((status == 1) || (status == 2));
    }
    
    bool GetAvailableUpdate(const std::string &tag) {
        if (tag.empty())
            return false;
            
        int current_ver = ((VERSION_MAJOR * 100) + (VERSION_MINOR * 10) + VERSION_MICRO);
        
        std::string tag_name = tag;
        tag_name.erase(std::remove_if(tag_name.begin(), tag_name.end(), [](char c) { return c == '.'; }), tag_name.end());
        int available_ver = std::stoi(tag_name);
        return (available_ver > current_ver);
    }
    
    size_t WriteJSONData(const char *ptr, size_t size, size_t nmemb, void *userdata) {
        const size_t total_size(size * nmemb);
        reinterpret_cast<std::string *>(userdata)->append(ptr, total_size);
        return total_size;
    }
    
    std::string GetLatestReleaseJSON(void) {
        std::string json = std::string();
        CURL *handle = curl_easy_init();
        
        curl_slist *header_data = nullptr;
        header_data = curl_slist_append(header_data, "Content-Type: application/json");
        header_data = curl_slist_append(header_data, "Accept: application/json");
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, header_data);
        
        curl_easy_setopt(handle, CURLOPT_URL, "https://api.github.com/repos/joel16/3DShell/releases/latest");
        curl_easy_setopt(handle, CURLOPT_USERAGENT, "3DShell");
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, Net::WriteJSONData);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &json);
        curl_easy_perform(handle);
        curl_easy_cleanup(handle);
        
        json_t *root;
        json_error_t error;
        root = json_loads(json.c_str(), JSON_DECODE_ANY, &error);
        
        if (!root) {
            Log::Error("json_loads failed on line %d: %s\n", error.line, error.text);
            return std::string();
        }
        
        json_t *tag = json_object_get(root, "tag_name");
        std::string tag_name = json_string_value(tag);
        return tag_name;
    }
    
    size_t Write3dsxData(const char *ptr, size_t size, size_t nmemb, Handle *userdata) {
        if (R_SUCCEEDED(FSFILE_Write(*userdata, nullptr, offset, ptr, (size * nmemb), FS_WRITE_FLUSH)))
            offset += (size * nmemb);
        
        return (size * nmemb);
    }
    
    void GetLatestRelease3dsx(const std::string &tag) {
        Result ret = 0;
        Handle file;
        std::string path = "/3ds/3DShell/3DShell_UPDATE.3dsx";
        
        if (!FS::FileExists(sdmc_archive, path))
            FSUSER_CreateFile(sdmc_archive, fsMakePath(PATH_ASCII, path.c_str()), 0, 0);
        
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmc_archive, fsMakePath(PATH_ASCII, path.c_str()), FS_OPEN_WRITE, 0))) {
            Log::Error("fsFsOpenFile(%s) failed: 0x%x\n", path, ret);
            return;
        }
        
        CURL *handle = curl_easy_init();
        if (handle) {
            std::string URL = "https://github.com/joel16/3DShell/releases/download/" + tag + "/3DShell.3dsx";
            curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());
            curl_easy_setopt(handle, CURLOPT_USERAGENT, "3DShell");
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, Net::Write3dsxData);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, &file);
            curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
            curl_easy_perform(handle);
            curl_easy_cleanup(handle);
        }
        
        FSFILE_Close(file);
        offset = 0;
        return;
    }
}
