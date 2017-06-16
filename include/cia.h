#ifndef CIA_H
#define CIA_H

#include <3ds.h>
#include <stdlib.h>

Result removeTitle(u64 titleID, FS_MediaType media);
Result installCIA(const char * path, FS_MediaType media, bool update);

#endif