#ifndef _3D_SHELL_HBLOADER_H
#define _3D_SHELL_HBLOADER_H

#include <3ds.h>
#include <string>
#include <vector>

#define HBLOADER_PATH_PREFIX "sdmc:"

typedef struct HBLoaderArgBuf {
    u32 buffer[0x100];
    u32 length;
} HBLoaderArgBuf;

namespace HBLoader {
    void Init(void);
    bool Boot(const std::string &path, const std::string &args = std::string());
    bool PendingExit(void);
    bool Available(void);
}

#endif
