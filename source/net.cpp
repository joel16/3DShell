#include <3ds.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <jansson.h>
#include <regex>

#include "fs.h"
#include "log.h"
#include "net.h"

s64 downloadOffset = 0, downloadSize = 1;
bool downloadProgress = false;

namespace Net {
    static s64 offset = 0;
    static void *socbuffer = nullptr;

    static int ProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        downloadOffset = dlnow;
        downloadSize = dltotal;
        return 0;
    }

    Result Init(void) {
        Result ret = 0;
        socbuffer = aligned_alloc(0x1000, 0x100000);

        if (!socbuffer) {
            return -1;
        }
            
        if (R_FAILED(ret = socInit(static_cast<u32* >(socbuffer), 0x100000))) {
            Log::Error("socInit() failed: 0x%x\n", ret);
            std::free(socbuffer);
            return ret;
        }

        return 0;
    }

    void Exit(void) {
        socExit();

        if (socbuffer) {
            std::free(socbuffer);
        }
    }
    
    bool GetNetworkStatus(void) {
        Result ret = 0;
        u32 status = 0;

        if (R_FAILED(ret = ACU_GetStatus(&status))) {
            Log::Error("ACU_GetStatus() failed: 0x%x\n", ret);
            return false;
        }

        return (status == 3);
    }
    
    bool GetAvailableUpdate(const std::string &tag) {
        if (tag.empty()) {
            return false;
        }
            
        int current_ver = ((VERSION_MAJOR * 100) + (VERSION_MINOR * 10) + VERSION_MICRO);
        
        std::string tag_name = tag;
        tag_name.erase(std::remove_if(tag_name.begin(), tag_name.end(), [](char c) { return c == '.'; }), tag_name.end()); // Remove decimal points
        tag_name = std::regex_replace(tag_name, std::regex(R"([\D])"), ""); // Remove any non numeric characters using regex
        int available_ver = std::stoi(tag_name); // Check if current version is lower than available version
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
        
        curl_slist *headerData = nullptr;
        headerData = curl_slist_append(headerData, "Content-Type: application/json");
        headerData = curl_slist_append(headerData, "Accept: application/json");
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headerData);
        
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
        std::string tagName = json_string_value(tag);
        return tagName;
    }
    
    size_t Write3dsxData(const char *ptr, size_t size, size_t nmemb, Handle *userdata) {
        if (R_SUCCEEDED(FSFILE_Write(*userdata, nullptr, offset, ptr, (size * nmemb), FS_WRITE_FLUSH))) {
            offset += (size * nmemb);
        }
        
        return (size * nmemb);
    }
    
    void GetLatestRelease(const std::string &tag) {
        Result ret = 0;
        Handle file;
        bool is3DSX = envIsHomebrew();
        const std::u16string path = (is3DSX? u"/3ds/3DShell/3DShell_UPDATE.3dsx" : u"/3ds/3DShell/3DShell_UPDATE.cia");
        
        if (!FS::FileExists(sdmcArchive, path)) {
            FSUSER_CreateFile(sdmcArchive, fsMakePath(PATH_UTF16, path.c_str()), 0, 0);
        }
        
        if (R_FAILED(ret = FSUSER_OpenFile(&file, sdmcArchive, fsMakePath(PATH_UTF16, path.c_str()), FS_OPEN_WRITE, 0))) {
            Log::Error("fsFsOpenFile(%s) failed: 0x%x\n", path, ret);
            return;
        }
        
        CURL *handle = curl_easy_init();
        if (handle) {
            std::string URL = "https://github.com/joel16/3DShell/releases/download/" + tag + (is3DSX? "/3DShell.3dsx" : "/3DShell.cia");
            curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());
            curl_easy_setopt(handle, CURLOPT_USERAGENT, "3DShell");
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, Net::ProgressCallback);
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
