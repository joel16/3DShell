#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <3ds.h>

Result Archive_ExtractZIP(const char *src);
Result Archive_ExtractRAR(const char *src);

#endif
