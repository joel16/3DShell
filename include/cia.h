#ifndef CIA_H
#define CIA_H

#include <3ds.h>
#include <stdlib.h>

#pragma GCC diagnostic ignored "-Wstrict-aliasing" // Ignore this one warning cause fuck it.

#define TEXTURE_CIA_LARGE_ICON 51

// App description
typedef struct 
{
	u16 shortDescription[0x40];
	u16 longDescription[0x80];
	u16 publisher[0x40];
} SMDH_title;


// Application settings
typedef struct 
{
	u8 ratings[0x10];
	u32 region;
	u32 matchMakerId;
	u64 matchMakerBitId;
	u32 flags;
	u16 eulaVersion;
	u16 reserved;
	u32 optimalBannerFrame;
	u32 streetpassId;
} SMDH_settings;

// Region specific game age ratings
typedef enum 
{
	RATING_CERO = 0, // Japan
	RATING_ESRB = 1, // USA
	RATING_RESERVED_0 = 2,
	RATING_USK = 3, // German
	RATING_PEGI_GEN = 4, // Europe
	RATING_RESERVED_1 = 5,
	RATING_PEGI_PRT = 6, // Portugal
	RATING_PEGI_BBFC = 7, // England
	RATING_COB = 8, // Australia
	RATING_GRB = 9, // South Korea
	RATING_CGSRR = 10, // Taiwan
	RATING_RESERVED_2 = 11,
	RATING_RESERVED_3 = 12,
	RATING_RESERVED_4 = 13,
	RATING_RESERVED_5 = 14,
	RATING_RESERVED_6 = 15
} SMDH_rating;

// Region lockout
typedef enum 
{
	REGION_JAPAN = 0x01,
	REGION_NORTH_AMERICA = 0x02,
	REGION_EUROPE = 0x04,
	REGION_AUSTRALIA = 0x08,
	REGION_CHINA = 0x10,
	REGION_KOREA = 0x20,
	REGION_TAIWAN = 0x40,
	REGION_FREE = 0x7FFFFFFF // Nintendo defines region free as 0x7FFFFFFF
} SMDH_region;

// Flags
typedef enum 
{
	FLAG_VISIBILITY = 0x0001,
	FLAG_AUTOBOOT = 0x0002,
	FLAG_ALLOW_3D = 0x0004,
	FLAG_REQUIRE_EULA = 0x0008,
	FLAG_SAVE_ON_EXIT = 0x0010,
	FLAG_USE_EXTENDED_BANNER = 0x0020,
	FLAG_RATING_REQUIRED = 0x0040,
	FLAG_USE_SAVE_DATA = 0x0080,
	FLAG_RECORD_USAGE = 0x0100,
	FLAG_DISABLE_SAVE_BACKUPS = 0x0400
} SMDH_flag;

// Format
typedef struct 
{
	char magic[0x04];
	u16 version;
	u16 reserved1;
	SMDH_title titles[0x10];
	SMDH_settings settings;
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
	char productCode[0x10];
	char title[0x40];
	char description[0x80];
	char author[0x40];
	u16 version;
	u32 coreVersion;
	u64 size;
	u64 requiredSize;
} Cia;

Result installCIA(const char * path, FS_MediaType media, bool update);
Result displayCIA(const char * path);

#endif