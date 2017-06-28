#ifndef CIA_H
#define CIA_H

#include <3ds.h>
#include <stdlib.h>

#pragma GCC diagnostic ignored "-Wstrict-aliasing" // Ignore this one warning cause fuck it.
 
typedef enum 
{
	PLATFORM_WII,
	PLATFORM_DSI,
	PLATFORM_3DS,
	PLATFORM_WIIU,
	PLATFORM_UNKNOWN
} AppPlatform;

typedef enum 
{
	CATEGORY_APP,
	CATEGORY_DEMO,
	CATEGORY_DLC,
	CATEGORY_PATCH,
	CATEGORY_SYSTEM,
	CATEGORY_TWL
} AppCategory;

typedef struct 
{
	u64 titleID;
	u32 uniqueID;
	FS_MediaType mediaType;
	AppPlatform platform;
	AppCategory category;
	u16 version;
	u64 size;
} Cia;

Result installCIA(const char * path, FS_MediaType media, bool update);
int displayCIA(const char * path);

#endif