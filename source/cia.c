#include "cia.h"
#include "clock.h"
#include "common.h"
#include "language.h"
#include "power.h"
#include "screenshot.h"
#include "utils.h"
#include "wifi.h"

sf2d_texture * largeIcon;

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

SMDH getCiaSMDH(const char * cia)
{
	SMDH smdh;

	FILE * file = fopen(cia, "rb");
	if(!file)
		return smdh;

	if(fseek(file, -0x36C0, SEEK_END) != 0)
	{
		fclose(file);
		return smdh;
	}

	size_t bytesRead = fread(&smdh, sizeof(SMDH), 1, file);
	if(bytesRead < 0)
	{
		fclose(file);
		return smdh;
	}

	fclose(file);

	if(smdh.magic[0] != 'S' || smdh.magic[1] != 'M' || smdh.magic[2] != 'D' || smdh.magic[3] != 'H')
		return smdh;
	
	return smdh;
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
	
	// Got the Bitmap stuff from lpp-3DS by Rinnegatamante, from here:
	char* largeIconData = (char*)malloc(0x36C0);
	ret = AM_GetCiaIcon((void*)largeIconData, fileHandle);
	char * buffer = (char*)&largeIconData[0x24C0];
	u16* icon_buffer = (u16*)buffer;
	
	Bitmap * bitmap = (Bitmap*)malloc(sizeof(Bitmap));
	bitmap->width = 48;
	bitmap->height = 48;
	bitmap->pixels = (u8*)malloc(6912);
	bitmap->bitperpixel = 24;
	
	//convert RGB565 to RGB24
	int tile_size = 16;
	int tile_number = 1;
	int extra_x = 0;
	int extra_y = 0;
	int i = 0;
	int tile_x[16] = {0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3};
	int tile_y[16] = {0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3};
	
	while (tile_number < 37)
	{
		while (i < (tile_size))
		{
			putPixel565(bitmap->pixels, tile_x[i-((tile_number - 1)<<6)] + extra_x, tile_y[i-((tile_number - 1)<<6)] + extra_y, icon_buffer[i]);
			putPixel565(bitmap->pixels, 4+tile_x[i-((tile_number - 1)<<6)] + extra_x, tile_y[i-((tile_number - 1)<<6)] + extra_y, icon_buffer[i + 16]);
			putPixel565(bitmap->pixels, tile_x[i-((tile_number - 1)<<6)] + extra_x, 4 + tile_y[i-((tile_number - 1)<<6)] + extra_y, icon_buffer[i + 32]);
			putPixel565(bitmap->pixels, 4+tile_x[i-((tile_number - 1)<<6)] + extra_x, 4 + tile_y[i-((tile_number - 1)<<6)] + extra_y, icon_buffer[i + 48]);
			i++;
		}
			
		if (tile_number % 6 == 0)
		{
			extra_x = 0;
			extra_y = extra_y + 8;
		}
		else 
			extra_x = extra_x + 8;
		
		tile_number++;
		tile_size = tile_size + 64;
		i = i + 48;
	}
	
	bitmap->magic = 0x4C494D47;
	
	u8 * real_pixels = 0;
	u8 * flipped = (u8*)malloc(bitmap->width * bitmap->height * (bitmap->bitperpixel >> 3));
	flipped = flipBitmap24(flipped, bitmap);
	int length = (bitmap->width * bitmap->height) << 2;
	
	if (bitmap->bitperpixel == 24)
	{		
		real_pixels = (u8*)malloc(length);
		int i = 0;
		int z = 0;
		
		while (i < length)
		{
			real_pixels[i] = flipped[z+2];
			real_pixels[i+1] = flipped[z+1];
			real_pixels[i+2] = flipped[z];
			real_pixels[i+3] = 0xFF;
			i = i + 4;
			z = z + 3;
		}
		
		free(flipped);
	}
	
	largeIcon = sf2d_create_texture_mem_RGBA8(real_pixels, bitmap->width, bitmap->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	
	free(real_pixels); // To here.
	
	FSFILE_Close(fileHandle);
	
	cia.titleID = info.titleID;
	cia.uniqueID = ((u32*) &info.titleID)[0];
	cia.mediaType = mediaType;
	cia.platform = platformFromId(((u16*) &info.titleID)[3]);
	cia.category = categoryFromId(((u16*) &info.titleID)[2]);
	cia.version = info.version;
	cia.size = info.size;																																															
	
	SMDH smdh = getCiaSMDH(path);
	
	if(smdh.titles != NULL)
	{
		char buffer[512];
		if(smdh.titles[language].shortDescription != NULL)
		{
			memset(buffer, 0, 0x40);
			utfn2ascii(buffer, smdh.titles[language].shortDescription, 0x40);
			strcpy(cia.title, buffer);
		}
		if(smdh.titles[language].longDescription != NULL)
		{
			memset(buffer, 0, 0x80);
			utfn2ascii(buffer, smdh.titles[language].longDescription, 0x80);
			strcpy(cia.description, buffer);
		}
		if(smdh.titles[language].publisher != NULL)
		{
			memset(buffer, 0, 0x40);
			utfn2ascii(buffer, smdh.titles[language].publisher, 0x40);
			strcpy(cia.author, buffer);
		}
	}
	
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
		
		sf2d_draw_texture(largeIcon, 15, 28);
		sftd_draw_textf(font, 78, 28, RGBA8(0, 0, 0, 255), 11, "%s v%u (0x%016llX)", fileName, cia.version, cia.titleID);
		sftd_draw_textf(font, 78, 44, RGBA8(0, 0, 0, 255), 11, "%s %s by %s", platformString(cia.platform), categoryString(cia.category), cia.author);
		sftd_draw_textf(font, 78, 60, RGBA8(0, 0, 0, 255), 11, "%s", size);
		
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
	sf2d_free_texture(largeIcon);
	return 0;
}