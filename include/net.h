#ifndef _3D_SHELL_NET_H
#define _3D_SHELL_NET_H

#include <string>

namespace Net {
    Result Init(void);
    void Exit(void);
    bool GetNetworkStatus(void);
    bool GetAvailableUpdate(const std::string &tag);
    std::string GetLatestReleaseJSON(void);
    void GetLatestRelease(const std::string &tag);
}

#endif
