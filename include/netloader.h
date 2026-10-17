#ifndef _3D_SHELL_NETLOADER_H
#define _3D_SHELL_NETLOADER_H

#define NETLOADER_PORT 17491

typedef enum NetLoaderResult {
    NL_CANCELLED = 0,
    NL_RECEIVED = 1,
    NL_ERROR = -1,
    NL_NOCHAIN = 2,
} NetLoaderResult;

namespace NetLoader {
    NetLoaderResult Run(void);
}

#endif
