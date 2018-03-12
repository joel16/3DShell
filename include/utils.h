#ifndef UTILS_H
#define UTILS_H

#include <3ds.h>

void Utils_MakeDirectories(void);
Result Utils_SaveConfig(int sortBy, bool recycleBin, bool protection, bool hidden);
Result Utils_LoadConfig(void);
Result Utils_GetLastDirectory(void);
char * Utils_Basename(const char * filename);
void Utils_GetSizeString(char * string, uint64_t size);
CFG_Region Utils_GetRegion(void);
CFG_Language Utils_GetLanguage(void);
const char * Utils_GetUsername(void);
bool Utils_IsN3DS(void);
void u16_to_u8(char * buf, const u16 * input, size_t bufsize);

#endif