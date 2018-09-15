#ifndef _3DSHELL_LOG_H
#define _3DSHELL_LOG_H

#define DEBUG(...) log_func(__VA_ARGS__)

void log_func(const char *s, ...);

#endif
