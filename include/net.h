#ifndef _3D_SHELL_NET_H
#define _3D_SHELL_NET_H

#include <string>

extern s64 download_offset, download_size;
extern bool download_progress;

namespace Net {
    Result Init(void);
    void Exit(void);
    bool GetNetworkStatus(void);
    bool GetAvailableUpdate(const std::string &tag);
    std::string GetLatestReleaseJSON(void);
    void GetLatestRelease(const std::string &tag);
}

#endif
