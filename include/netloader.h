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
    /* hbmenu-style worker thread: the panel draws progress from these
     * accessors while the transfer runs on its own thread. */
    void Start(void);
    void Cancel(void);
    bool Busy(void);
    s64 Total(void);
    s64 Size(void);
    const char *Message(void);
}

#endif
