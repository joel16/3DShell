#include "cia.h"
#include "clock.h"
#include "common.h"
#include "power.h"
#include "screenshot.h"
#include "utils.h"
#include "wifi.h"

const char * platformString(AppPlatform platform) 
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

const char * categoryString(AppCategory category) 
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

AppPlatform platformFromId(u16 id)
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

AppCategory categoryFromId(u16 id) 
{
	if((id & 0x8000) == 0x8000) 
		return CATEGORY_TWL;
	else if((id & 0x10) == 0x10)
		return CATEGORY_SYSTEM;
	else if((id & 0x6) == 0x6) 
		return CATEGORY_PATCH;
	else if((id & 0x4) == 0x4) 
		return CATEGORY_DLC;
	else if((id & 0x2) == 0x2) 
		return CATEGORY_DEMO;
	
	return CATEGORY_APP;
}

Cia getCiaInfo(const char * path, FS_MediaType mediaType)
{
	Handle fileHandle;
	AM_TitleEntry info;
	Cia cia;
	
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_ASCII, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
	
	ret = AM_GetCiaFileInfo(mediaType, &info, fileHandle);
	
	if (ret)
		return cia;
	
	FSFILE_Close(fileHandle);
	
	cia.titleID = info.titleID;
	cia.uniqueID = ((u32*) &info.titleID)[0];
	cia.mediaType = mediaType;
	cia.platform = platformFromId(((u16*) &info.titleID)[3]);
	cia.category = categoryFromId(((u16*) &info.titleID)[2]);
	cia.version = info.version;
	cia.size = info.size;
	
	return cia;
}

Result removeTitle(u64 titleID, FS_MediaType media)
{
	u32 count = 0;
	Result ret = AM_GetTitleCount(media, &count);
	
	if (ret) 
		return ret;
	
	u32 read = 0;
	u64 * titleIDs = malloc(count * sizeof(u64));
	
	ret = AM_GetTitleList(&read, media, count, titleIDs);
	
	if (ret)
		return ret;
	
	for (unsigned int i = 0; i < read; i++) 
	{
		if (titleIDs[i] == titleID) 
		{
			ret = AM_DeleteAppTitle(media, titleID);
			break;
		}
	}
	
	free(titleIDs);
	
	if (ret) 
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
	
	ret = AM_GetCiaFileInfo(media, &title, fileHandle);
	
	if (!update) // As long as we aren't updating 3DShell, remove the title before installing.
	{
		ret = removeTitle(title.titleID, media);
		if (ret) 
			return ret;
	}
	
	ret = FSFILE_GetSize(fileHandle, &size);
	
	ret = AM_StartCiaInstall(media, &ciaHandle);
	
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

	ret = svcCloseHandle(fileHandle);
	
	if (ret)
		return ret;
	
	return 2;
}

void launchCIA(u64 titleId, FS_MediaType mediaType)
{
	u8 param[0x300];
    u8 hmac[0x20];
	
	APT_PrepareToDoApplicationJump(0, titleId, mediaType);
	APT_DoApplicationJump(param, sizeof(param), hmac);
}

int displayCIA(const char * path)
{
	/*sf2d_texture * smdhIcon = NULL;
	char* smdh_data = (char*)malloc(0x36C0);
	
	Handle fileHandle;
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_ASCII, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
	
	ret = AM_GetCiaIcon(smdh_data, fileHandle);
	
	smdhIcon = sfil_load_BMP_buffer(smdh_data, SF2D_PLACE_RAM);
	setBilinearFilter(smdhIcon);
	
	if (ret)
		return ret;*/
	
	bool update;
	
	if (strncmp(fileName, "3DShell.cia", 11) == 0)
		update = true;
	
	sf2d_set_clear_color(RGBA8(245, 245, 245, 255));
	
	int pBar = 34, xlim = 300;
	
	int isInstalling = 0;
	
	Cia cia = getCiaInfo(path, MEDIATYPE_SD);
	
	char size[16];
	getSizeString(size, cia.size);
		
	while (aptMainLoop())
	{
		hidScanInput();
		hidTouchRead(&touch);
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT); // Clear bottom screen
		
		sf2d_draw_rectangle(0, 0, 400, 240, RGBA8(250, 250, 250, 255));
		
		if (isInstalling == 0)
		{
			sftd_draw_text(font, (300 - sftd_get_text_width(font, 11, "INSTALL")), 220, RGBA8(0, 150, 136, 255), 11, "INSTALL");
			sftd_draw_text(font, (300 - (sftd_get_text_width(font, 11, "CANCEL") + sftd_get_text_width(font, 11, "INSTALL") + 20)), 220, RGBA8(0, 150, 136, 255), 11, "CANCEL");
		}
		else if (isInstalling == 2)
		{
			sftd_draw_text(font, (300 - sftd_get_text_width(font, 11, "OPEN")), 220, RGBA8(0, 150, 136, 255), 11, "OPEN");
			sftd_draw_text(font, (300 - (sftd_get_text_width(font, 11, "DONE") + sftd_get_text_width(font, 11, "OPEN") + 20)), 220, RGBA8(0, 150, 136, 255), 11, "DONE");
		}
		
		sf2d_end_frame();
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		
		sf2d_draw_rectangle(0, 0, 400, 16, RGBA8(117, 117, 117, 255));
		
		drawWifiStatus(270, 2);
		drawBatteryStatus(295, 2);
		digitalTime(346, 1);
		
		//sf2d_draw_texture(smdhIcon, 30, 28);
		sftd_draw_textf(font, 78, 28, RGBA8(0, 0, 0, 255), 11, "%s v%u (%u)", fileName, cia.version, cia.uniqueID);
		sftd_draw_textf(font, 78, 44, RGBA8(0, 0, 0, 255), 11, "%s %s", platformString(cia.platform), categoryString(cia.category));
		sftd_draw_textf(font, 78, 60, RGBA8(0, 0, 0, 255), 11, "%s", size);
		
		sf2d_draw_rectangle(15, 28, 48, 48, RGBA8(70, 70, 70, 255));
		
		if (isInstalling == 0)
			sftd_draw_text(font, 15, 86, RGBA8(0, 0, 0, 255), 11, "Do you want to install this application?");
		
		else if (isInstalling == 1)
		{
			sf2d_draw_rectangle(100, 130, 200, 3, RGBA8(185, 224, 220, 255));
			sf2d_draw_rectangle(pBar, 130, 66, 3, RGBA8(0, 150, 136, 255));
		
			// Boundary stuff
			sf2d_draw_rectangle(0, 130, 100, 3, RGBA8(245, 245, 245, 255));
			sf2d_draw_rectangle(300, 130, 66, 3, RGBA8(245, 245, 245, 255)); 
		
			sftd_draw_text(font, ((400 - sftd_get_text_width(font, 11, "Installing...")) / 2), 146, RGBA8(0, 0, 0, 255), 11, "Installing...");
		
			pBar += 4;
		
			if (pBar >= xlim)
				pBar = 34;
		}
		else
			sftd_draw_text(font, ((400 - sftd_get_text_width(font, 11, "App installed.")) / 2), 146, RGBA8(0, 0, 0, 255), 11, "App installed.");
		
		endDrawing();
		
		if (isInstalling == 0)
		{
			if (touchInRect((300 - sftd_get_text_width(font, 11, "INSTALL")), 300, 220, 240))
			{
				isInstalling = 1;
				isInstalling = installCIA(path, MEDIATYPE_SD, update);
				//wait(100000000);
			}
			else if (touchInRect((300 - (sftd_get_text_width(font, 11, "CANCEL") + sftd_get_text_width(font, 11, "INSTALL") + 20)), ((300 - 20) - sftd_get_text_width(font, 11, "INSTALL")), 220, 240))
			{
				wait(100000000);
				break;
			}
		}
		else if (isInstalling == 2)
		{
			if (touchInRect((300 - sftd_get_text_width(font, 11, "OPEN")), 300, 220, 240))
			{
				wait(100000000);
				launchCIA(cia.titleID, cia.mediaType);
			}
			else if (touchInRect((300 - (sftd_get_text_width(font, 11, "DONE") + sftd_get_text_width(font, 11, "OPEN") + 20)), ((300 - 20) - sftd_get_text_width(font, 11, "OPEN")), 220, 240))
			{
				wait(100000000);
				break;
			}
		}
				
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}
	
	//delete smdhIcon
	//sf2d_free_texture(smdhIcon);
	return 0;
}