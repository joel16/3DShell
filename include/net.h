#pragma once

#include <string>

extern s64 downloadOffset, downloadSize;
extern bool downloadProgress;

namespace Net {
    Result Init(void);
    void Exit(void);
    bool GetNetworkStatus(void);
    bool GetAvailableUpdate(const std::string &tag);
    std::string GetLatestReleaseJSON(void);
    void GetLatestRelease(const std::string &tag);
}
