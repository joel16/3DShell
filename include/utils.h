#ifndef _3D_SHELL_UTILS_H
#define _3D_SHELL_UTILS_H

bool Utils_IsN3DS(void);
void Utils_U16_To_U8(char *buf, const u16 *input, size_t bufsize);
char *Utils_Basename(const char *filename);
void Utils_GetSizeString(char *string, u64 size);
void Utils_SetMax(int *set, int value, int max);
void Utils_SetMin(int *set, int value, int min);
int Utils_Alphasort(const void *p1, const void *p2);
void Utils_AppendArr(char subject[], const char insert[], int pos);

#endif