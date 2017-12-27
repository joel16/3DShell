#ifndef NET_H
#define NET_H

#define nullptr 0

#include <3ds.h>

char dl_url[255];
u32 dl_size;
Result downloadFile(const char * url, const char * path);

#endif