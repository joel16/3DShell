#ifndef UTILS_H
#define UTILS_H

#include <sf2d.h>
#include <stdlib.h>

#include <png.h>

#define SDK(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)

#define touchInRect(x1, x2, y1, y2) ((touchGetX() >= (x1) && touchGetX() <= (x2)) && (touchGetY() >= (y1) && touchGetY() <= (y2)))

#define wait(nanoSec) svcSleepThread(nanoSec);

typedef struct 
{
	u32 magic;
	u8* pixels;
	int width;
	int height;
	u16 bitperpixel;
} Bitmap;

void setBilinearFilter(sf2d_texture *texture);
void endDrawing();
void getSizeString(char * string, uint64_t size);
int touchGetX();
int touchGetY();
sf2d_texture *sfil_load_IMG_file(const char * filename, sf2d_place place);
int extractZip(const char * zipFile, const char * path);
void setConfig(const char * path, bool set);
const char * getLastNChars(char * str, int n);
u8 getRegion();
u8 getLanguage();
const char * getUsername();
bool isN3DS();
void utf2ascii(char * dst, u16* src);
void utfn2ascii(char * dst, u16* src, int max);
void putPixel565(u8 * dst, u8 x, u8 y, u16 v);
u8* flipBitmap24(u8 * flip_bitmap, Bitmap * result);

#endif