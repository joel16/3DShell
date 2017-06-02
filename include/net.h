#ifndef NET_H
#define NET_H

#define nullptr 0

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Result downloadFile(const char * url, const char * path);

#endif