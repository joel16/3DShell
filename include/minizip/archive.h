#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <3ds.h>

Result Archive_ExtractZIP(const char * src, const char * dst);
Result Archive_ExtractRAR(const char *src, const char *dst);

#endif