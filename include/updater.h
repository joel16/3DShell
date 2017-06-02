#ifndef UPDATER_H
#define UPDATER_H

#define nullptr 0

#include <3ds.h>

#define BUFFERSIZE (1024 * 128)

char * checkForUpdate();
void downloadUpdate();
void installUpdate();

#endif