#ifndef _3D_SHELL_NET_H
#define _3D_SHELL_NET_H

#include <string>

namespace Net {
    Result Init(void);
    void Exit(void);
    bool GetNetworkStatus(void);
    bool GetAvailableUpdate(const std::string &tag);
    std::string GetLatestReleaseJSON(void);
    void GetLatestRelease3dsx(const std::string &tag);
}

#endif
