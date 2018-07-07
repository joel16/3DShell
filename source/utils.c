#include <stdio.h>
#include <string.h>

#include <3ds.h>

#include "utils.h"

bool Utils_IsN3DS(void)
{
	bool isNew3DS = false;

	if (R_SUCCEEDED(APT_CheckNew3DS(&isNew3DS)))
		return isNew3DS;

	return false;
}

void Utils_U16_To_U8(char *buf, const u16 *input, size_t bufsize)
{
	ssize_t units = utf16_to_utf8((u8 *)buf, input, bufsize);

	if (units < 0)
		units = 0;
	
	buf[units] = 0;
}

char *Utils_Basename(const char *filename)
{
	char *p = strrchr (filename, '/');
	return p ? p + 1 : (char *) filename;
}

void Utils_GetSizeString(char *string, u64 size)
{
	double double_size = (double)size;

	int i = 0;
	static char *units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

	while (double_size >= 1024.0f)
	{
		double_size /= 1024.0f;
		i++;
	}

	sprintf(string, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

void Utils_SetMax(int *set, int value, int max)
{
	if (*set > max)
		*set = value;
}

void Utils_SetMin(int *set, int value, int min)
{
	if (*set < min)
		*set = value;
}

int Utils_Alphasort(const void *p1, const void *p2)
{
	FS_DirectoryEntry* entryA = (FS_DirectoryEntry*) p1;
	FS_DirectoryEntry* entryB = (FS_DirectoryEntry*) p2;

	if ((entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && !(entryB->attributes & FS_ATTRIBUTE_DIRECTORY))
		return -1;
	else if (!(entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && (entryB->attributes & FS_ATTRIBUTE_DIRECTORY))
		return 1;
	
	char entryNameA[256] = {'\0'}, entryNameB[256] = {'\0'};
	Utils_U16_To_U8((u8 *) entryNameA, entryA->name, sizeof(entryNameA) - 1);
	Utils_U16_To_U8((u8 *) entryNameB, entryB->name, sizeof(entryNameB) - 1);
	return strcasecmp(entryNameA, entryNameB);
}

void Utils_AppendArr(char subject[], const char insert[], int pos)
{
	char buf[100] = {}; // 100 so that it's big enough. fill with 0
	// or you could use malloc() to allocate sufficient space
	
	strncpy(buf, subject, pos); // copy at most first pos characters
	int len = strlen(buf);
	strcpy(buf+len, insert); // copy all of insert[] at the end
	len += strlen(insert);  // increase the length by length of insert[]
	strcpy(buf+len, subject+pos); // copy the rest
	
	strcpy(subject, buf);   // copy it back to subject
	// deallocate buf[] here, if used malloc()
}