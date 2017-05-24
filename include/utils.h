#ifndef UTILS_H
#define UTILS_H

#include <3ds.h>
#include <sf2d.h>
#include <stdio.h>

#define SDK(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)

#define touchInRect(x1, x2, y1, y2) ((touchGetX() >= (x1) && touchGetX() <= (x2)) && (touchGetY() >= (y1) && touchGetY() <= (y2)))

void setBilinearFilter(sf2d_texture *texture);
void endDrawing();
void getSizeString(char * string, uint64_t size);
int touchGetX();
int touchGetY();
int extractZip(const char * zipFile, const char * path);
void setBgm(bool set);

#endif