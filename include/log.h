#ifndef _3DSHELL_LOG_H
#define _3DSHELL_LOG_H

#include <3ds.h>

Result Log_OpenFileHande(void);
Result Log_CloseFileHandle(void);
Result Log_Print(const char *s, ...);

#endif
