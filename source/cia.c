#include "cia.h"
#include "clock.h"
#include "common.h"
#include "fs.h"
#include "language.h"
#include "power.h"
#include "screen.h"
#include "screenshot.h"
#include "utils.h"
#include "wifi.h"

static const char * platformString(AppPlatform platform) 
{
	switch(platform) 
	{
		case PLATFORM_WII:
			return "Wii";
		case PLATFORM_DSI:
			return "DSi";
		case PLATFORM_3DS:
			return "3DS";
		case PLATFORM_WIIU:
			return "Wii U";
		default:
			return "Unknown";
	}	
}

static const char * categoryString(AppCategory category) 
{
	switch(category) 
	{
		case CATEGORY_APP:
			return "App";
		case CATEGORY_DEMO:
			return "Demo";
		case CATEGORY_DLC:
			return "DLC";
		case CATEGORY_PATCH:
			return "Patch";
		case CATEGORY_SYSTEM:
			return "System";
		case CATEGORY_TWL:
			return "TWL";
		default:
			return "Unknown";
	}
}

static AppPlatform platformFromId(u16 id)
{
	switch(id) 
	{
		case 1:
			return PLATFORM_WII;
		case 3:
			return PLATFORM_DSI;
		case 4:
			return PLATFORM_3DS;
		case 5:
			return PLATFORM_WIIU;
		default:
			return PLATFORM_UNKNOWN;
    }
}

static AppCategory categoryFromId(u16 id) 
{
	if ((id & 0x8000) == 0x8000) 
		return CATEGORY_TWL;
	else if ((id & 0x10) == 0x10)
		return CATEGORY_SYSTEM;
	else if ((id & 0x6) == 0x6) 
		return CATEGORY_PATCH;
	else if ((id & 0x4) == 0x4) 
		return CATEGORY_DLC;
	else if ((id & 0x2) == 0x2) 
		return CATEGORY_DEMO;
	
	return CATEGORY_APP;
}

static Result parseSMDH(const char * cia, SMDH * smdh) 
{
	Handle handle;
	
	if (R_SUCCEEDED(fsOpen(&handle, cia, FS_OPEN_READ))) 
	{
		u32 bytesRead = 0;
		
		if (R_SUCCEEDED(FSFILE_Read(handle, &bytesRead, 0, smdh, sizeof(SMDH))) && bytesRead == sizeof(SMDH)) 
		{
			if (smdh->magic[0] == 'S' && smdh->magic[1] == 'M' && smdh->magic[2] == 'D' && smdh->magic[3] == 'H') 
			{
				return FSFILE_Close(handle);
			}
		}
		
		return FSFILE_Close(handle);
	}	
	
	return 0;
}

static Cia getCiaInfo(const char * path, FS_MediaType mediaType)
{
	Handle fileHandle;
	AM_TitleEntry titleInfo;
	Cia cia;
	
	Result ret = fsOpen(&fileHandle, path, FS_OPEN_READ);
	
	ret = AM_GetCiaFileInfo(mediaType, &titleInfo, fileHandle);
	
	ret = AM_GetCiaRequiredSpace(&cia.requiredSize, mediaType, fileHandle);
	
	ret = AM_GetCiaCoreVersion(&cia.coreVersion, fileHandle);
	
	if (R_FAILED(ret))
		return cia;
	
	cia.titleID = titleInfo.titleID;
	cia.uniqueID = ((u32 *) &titleInfo.titleID)[0];
	cia.mediaType = mediaType;
	cia.platform = platformFromId(((u16*) &titleInfo.titleID)[3]);
	cia.category = categoryFromId(((u16*) &titleInfo.titleID)[2]);
	cia.version = titleInfo.version;
	cia.size = titleInfo.size;																																															
	
	SMDH smdh;
	
	if (R_SUCCEEDED(parseSMDH(path, &smdh)))
	{		
		CFG_Language language;
		CFGU_GetSystemLanguage(&language);

		char * largeIconData = (char *)malloc(0x36C0);
		AM_GetCiaIcon((void *)largeIconData, fileHandle);
		char * buffer = (char *)&largeIconData[0x24C0];
			
		screen_load_texture_tiled(TEXTURE_CIA_LARGE_ICON, buffer, sizeof(smdh.largeIcon), 48, 48, GPU_RGB565, false);
		
		u16_to_u8(cia.title, smdh.titles[language].shortDescription, 0x41);
		u16_to_u8(cia.description, smdh.titles[language].longDescription, 0x80);
		u16_to_u8(cia.author, smdh.titles[language].publisher, 0x41);
	}
	
	FSFILE_Close(fileHandle);
	return cia;
}

static Result removeTitle(u64 titleID, FS_MediaType media)
{
	u32 count = 0;
	
	Result ret = AM_GetTitleCount(media, &count);
	if (R_FAILED(ret))
		return ret;
	
	u32 read = 0;
	u64 * titleIDs = malloc(count * sizeof(u64));
	
	ret = AM_GetTitleList(&read, media, count, titleIDs);
	if (R_FAILED(ret))
		return ret;
	
	for (unsigned int i = 0; i < read; i++) 
	{
		if (titleIDs[i] == titleID) 
		{
			ret = AM_DeleteAppTitle(media, titleID);
			if (R_FAILED(ret))
				return ret;
			break;
		}
	}
	
	free(titleIDs);
	
	if (R_FAILED(ret))
		return ret;
	
	return 0;
}

Result installCIA(const char * path, FS_MediaType media, bool update)
{
	u64 size = 0;
	u32 bytes;
	Handle ciaHandle, fileHandle;
	AM_TitleEntry title;
	u32 read = 0x1000;
	
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_ASCII, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
	if (R_FAILED(ret))
		return ret;
	
	ret = AM_GetCiaFileInfo(media, &title, fileHandle);
	if (R_FAILED(ret))
		return ret;
	
	if (!update) // As long as we aren't updating 3DShell, remove the title before installing.
	{
		ret = removeTitle(title.titleID, media);
		if (R_FAILED(ret))
			return ret;
	}
	
	ret = FSFILE_GetSize(fileHandle, &size);
	if (R_FAILED(ret))
		return ret;
	
	ret = AM_StartCiaInstall(media, &ciaHandle);
	if (R_FAILED(ret))
		return ret;
	
	u8 * cia_buffer = malloc(read);
	
	for (u64 startSize = size; size != 0; size -= read) 
	{
		if (size < read) 
			read = size;
		FSFILE_Read(fileHandle, &bytes, startSize-size, cia_buffer, read);
		FSFILE_Write(ciaHandle, &bytes, startSize-size, cia_buffer, read, 0);
	}
	
	free(cia_buffer);
	
	ret = AM_FinishCiaInstall(ciaHandle);
	if (R_FAILED(ret))
		return ret;

	ret = svcCloseHandle(fileHandle);
	if (R_FAILED(ret))
		return ret;
	
	return 2;
}

static void launchCIA(u64 titleId, FS_MediaType mediaType)
{
	u8 param[0x300];
	u8 hmac[0x20];
	
	APT_PrepareToDoApplicationJump(0, titleId, mediaType);
	APT_DoApplicationJump(param, sizeof(param), hmac);
}

Result displayCIA(const char * path)
{	
	bool update;
	
	if (strncmp(fileName, "3DShell.cia", 12) == 0)
		update = true;
	
	screen_clear(GFX_TOP, RGBA8(245, 245, 245, 255));
	screen_clear(GFX_BOTTOM, RGBA8(245, 245, 245, 255));
	
	int isInstalling = 0;
	
	Cia cia = getCiaInfo(path, MEDIATYPE_SD);
	
	char size[16];
	getSizeString(size, cia.size);
	
	/*char requiredSpace[16];
	getSizeString(size, cia.requiredSize);*/
	
	int pBar = 34, xlim = 300;
		
	while (aptMainLoop())
	{
		hidScanInput();
		hidTouchRead(&touch);
		
		screen_begin_frame();
		screen_select(GFX_BOTTOM);
		
		screen_draw_rect(0, 0, 400, 240, RGBA8(250, 250, 250, 255));
		
		if (isInstalling == 0)
		{
			screen_draw_string((300 - screen_get_string_width("INSTALL", 0.41f, 0.41f)), 220, 0.41f, 0.41f, RGBA8(0, 150, 136, 255), "INSTALL");
			screen_draw_string((300 - (screen_get_string_width("CANCEL", 0.41f, 0.41f) + screen_get_string_width("INSTALL", 0.41f, 0.41f) + 20)), 220, 0.41f, 0.41f, RGBA8(0, 150, 136, 255), "CANCEL");
		}
		else if (isInstalling == 2)
		{
			screen_draw_string((300 - screen_get_string_width("OPEN", 0.41f, 0.41f)), 220, 0.41f, 0.41f, RGBA8(0, 150, 136, 255), "OPEN");
			screen_draw_string((300 - (screen_get_string_width("DONE", 0.41f, 0.41f) + screen_get_string_width("OPEN", 0.41f, 0.41f) + 20)), 220, 0.41f, 0.41f, RGBA8(0, 150, 136, 255), "DONE");
		}
		
		screen_select(GFX_TOP);
		
		screen_draw_rect(0, 0, 400, 16, RGBA8(117, 117, 117, 255));
		
		drawWifiStatus(270, 2);
		drawBatteryStatus(295, 2);
		digitalTime();
		
		screen_draw_texture(TEXTURE_CIA_LARGE_ICON, 15, 28);
		screen_draw_stringf(78, 28, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "%s v%d.%d.%d (%016llX)", fileName, ((cia.version & 0xFC00) >> 10), ((cia.version & 0x03F0) >> 4), (cia.version & 0x000F), cia.titleID);
		screen_draw_stringf(78, 44, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "%s %s by %s", platformString(cia.platform), categoryString(cia.category), cia.author);
		screen_draw_stringf(78, 60, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "%s", size);
		
		if (isInstalling == 0)
		{
			screen_draw_string(15, 86, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "Do you want to install this application?");
			//screen_draw_stringf(15, 116, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "Program requires: %s", requiredSpace);	
		}
		else if (isInstalling == 1)
		{
			screen_draw_rect(100, 130, 200, 3, RGBA8(185, 224, 220, 255));
			screen_draw_rect(pBar, 130, 66, 3, RGBA8(0, 150, 136, 255));
		
			// Boundary stuff
			screen_draw_rect(0, 130, 100, 3, RGBA8(245, 245, 245, 255));
			screen_draw_rect(300, 130, 66, 3, RGBA8(245, 245, 245, 255)); 
	
			screen_draw_string(((400 - screen_get_string_width("Installing...", 0.41f, 0.41f)) / 2), 146, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "Installing...");
			
			pBar += 4;
		
			if (pBar >= xlim)
				pBar = 34;
			
			isInstalling = installCIA(path, MEDIATYPE_SD, update);
		}
		else
			screen_draw_string(((400 - screen_get_string_width("App installed.", 0.41f, 0.41f)) / 2), 146, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "App installed.");
		
		screen_end_frame();
		
		if (isInstalling == 0)
		{
			if (((touchInRect((300 - screen_get_string_width("INSTALL", 0.41f, 0.41f)), 300, 220, 240)) && (kPressed & KEY_TOUCH)) || (kPressed & KEY_A))
			{
				//wait(100000000);
				isInstalling = 1;
			}
			else if (((touchInRect((300 - (screen_get_string_width("CANCEL", 0.41f, 0.41f) + screen_get_string_width("INSTALL", 0.41f, 0.41f) + 20)), ((300 - 20) - screen_get_string_width("INSTALL", 0.41f, 0.41f)), 220, 240)) && (kPressed & KEY_TOUCH))  || (kPressed & KEY_B))
			{
				wait(100000000);
				break;
			}
		}
		else if (isInstalling == 2)
		{
			if (((touchInRect((300 - screen_get_string_width("OPEN", 0.41f, 0.41f)), 300, 220, 240)) && (kPressed & KEY_TOUCH)) || (kPressed & KEY_A))
			{
				//wait(100000000);
				launchCIA(cia.titleID, cia.mediaType);
			}
			else if (((touchInRect((300 - (screen_get_string_width("DONE", 0.41f, 0.41f) + screen_get_string_width("OPEN", 0.41f, 0.41f) + 20)), ((300 - 20) - screen_get_string_width("OPEN", 0.41f, 0.41f)), 220, 240)) && (kPressed & KEY_TOUCH)) || (kPressed & KEY_B))
			{
				wait(100000000);
				break;
			}
		}
				
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}
	
	// delete smdhIcon
	screen_unload_texture(TEXTURE_CIA_LARGE_ICON);
	return 0;
}