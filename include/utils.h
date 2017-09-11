#ifndef UTILS_H
#define UTILS_H

#include <3ds.h>

#define SDK(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)

#define touchInRect(x1, x2, y1, y2) ((touchGetX() >= (x1) && touchGetX() <= (x2)) && (touchGetY() >= (y1) && touchGetY() <= (y2)))

#define wait(msec) svcSleepThread(10000000 * (s64)msec)

void loadConfig(void);
void makeDirectories(void);
u64 getFreeStorage(FS_SystemMediaType mediaType);
u64 getTotalStorage(FS_SystemMediaType mediaType);
u64 getUsedStorage(FS_SystemMediaType mediaType);
void getSizeString(char * string, uint64_t size);
u16 touchGetX(void);
u16 touchGetY(void);
Result setConfig(const char * path, bool set);
const char * getLastNChars(char * str, int n);
u8 getRegion(void);
u8 getLanguage(void);
const char * getUsername(void);
bool isN3DS(void);
void u16_to_u8(char * buf, const u16 * input, size_t bufsize);

#endif