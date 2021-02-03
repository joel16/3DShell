#ifndef _3D_SHELL_LOG_H
#define _3D_SHELL_LOG_H

#include <3ds.h>

namespace Log {
    Result Open(void);
    Result Close(void);
    void Error(const char *data, ...);
}

#endif
