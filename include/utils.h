#ifndef UTILS_H
#define UTILS_H

#include <3ds.h>
#include <sf2d.h>
#include <stdio.h>
#include <stdlib.h>

#include <png.h>

#define SDK(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)

#define touchInRect(x1, x2, y1, y2) ((touchGetX() >= (x1) && touchGetX() <= (x2)) && (touchGetY() >= (y1) && touchGetY() <= (y2)))

void setBilinearFilter(sf2d_texture *texture);
void endDrawing();
void getSizeString(char * string, uint64_t size);
int touchGetX();
int touchGetY();
sf2d_texture *sfil_load_IMG_file(const char *filename, sf2d_place place);
int extractZip(const char * zipFile, const char * path);
void setConfig(char * path, bool set);
const char * getLastNChars(char * str, int n);
u8 getRegion();
u8 getLanguage();
const char * getUsername();
bool isN3DS();
int fastStrcmp(const char *ptr0, const char *ptr1, int len);

#endif