#ifndef CIA_H
#define CIA_H

#include <3ds.h>
#include <stdlib.h>

#pragma GCC diagnostic ignored "-Wstrict-aliasing" // Ignore this one warning cause fuck it.

typedef struct 
{
	u16 shortDescription[0x40];
	u16 longDescription[0x80];
	u16 publisher[0x40];
} SMDH_title;

typedef struct 
{
	char magic[0x04];
	u16 version;
	u16 reserved1;
	SMDH_title titles[0x10];
	u64 reserved2;
	u8 smallIcon[0x480];
	u8 largeIcon[0x1200];
} SMDH;
 
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
	char title[0x40];
	char description[0x80];
	char author[0x40];
	u16 version;
	u64 size;
} Cia;

Result installCIA(const char * path, FS_MediaType media, bool update);
int displayCIA(const char * path);

#endif