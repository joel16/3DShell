#include "cia.h"
#include "clock.h"
#include "common.h"
#include "language.h"
#include "power.h"
#include "screen.h"
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

void putPixel565(u8 * dst, u8 x, u8 y, u16 v)
{
	dst[(x+(47-y)*48)*3+0]=(v&0x1F)<<3;
	dst[(x+(47-y)*48)*3+1]=((v>>5)&0x3F)<<2;
	dst[(x+(47-y)*48)*3+2]=((v>>11)&0x1F)<<3;
}

u8* flipBitmap24(u8 * flip_bitmap, Bitmap * result)
{
	if (!result) 
		return NULL;
	
	int x, y;
	
	for (y = 0; y < result->height; y++)
	{
		for (x = 0; x < result->width; x++)
		{
			int idx = (x+y * result->width) * 3;
			*(u32*)(&(flip_bitmap[idx])) = ((*(u32*)&(result->pixels[(x + (result->height - y - 1) * result->width)*3]) & 0x00FFFFFF) | (*(u32*)(&(flip_bitmap[idx])) & 0xFF000000));
		}
	}
		
	return flip_bitmap;
}

Cia getCiaInfo(const char * path, FS_MediaType mediaType)
{
	Handle fileHandle;
	AM_TitleEntry info;
	Cia cia;
	
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_ASCII, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
	
	ret = AM_GetCiaFileInfo(mediaType, &info, fileHandle);
	//ret = AM_GetCiaRequiredSpace(&cia.requiredSpace, mediaType, fileHandle);
	ret = AM_GetCiaCoreVersion(&cia.coreVersion, fileHandle);
	
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
	
	// convert RGB565 to 24 Bitmap
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
	} // To here.
	
	for(u32 x = 0; x < bitmap->width; x++) 
	{
		for(u32 y = 0; y < bitmap->height; y++) 
		{
			u32 pos = (y * bitmap->width + x) * 4;

			u8 c1 = real_pixels[pos + 0];
			u8 c2 = real_pixels[pos + 1];
			u8 c3 = real_pixels[pos + 2];
			u8 c4 = real_pixels[pos + 3];

			real_pixels[pos + 0] = c4;
			real_pixels[pos + 1] = c3;
			real_pixels[pos + 2] = c2;
			real_pixels[pos + 3] = c1;
		}
	}
	
	screen_load_texture_untiled(TEXTURE_CIA_LARGE_ICON, real_pixels, (bitmap->width *  bitmap->height * 4), bitmap->width, bitmap->height, GPU_RGBA8, true);
	
	free(real_pixels);
	
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
			memset(buffer, 0, 64 + 1);
			utfn2ascii(buffer, smdh.titles[language].shortDescription, 64 + 1);
			strcpy(cia.title, buffer);
		}
		if(smdh.titles[language].longDescription != NULL)
		{
			memset(buffer, 0, 128 + 1);
			utfn2ascii(buffer, smdh.titles[language].longDescription, 128 + 1);
			strcpy(cia.description, buffer);
		}
		if(smdh.titles[language].publisher != NULL)
		{
			memset(buffer, 0, 64 + 1);
			utfn2ascii(buffer, smdh.titles[language].publisher, 64 + 1);
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
	
	screen_clear(GFX_TOP, RGBA8(245, 245, 245, 255));
	screen_clear(GFX_BOTTOM, RGBA8(245, 245, 245, 255));
	
	int isInstalling = 0;
	
	Cia cia = getCiaInfo(path, MEDIATYPE_SD);
	
	char size[16];
	getSizeString(size, cia.size);
	
	/*char requiredSpace[16];
	getSizeString(size, cia.requiredSpace);*/
	
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
		screen_draw_stringf(78, 28, 0.41f, 0.41f, RGBA8(0, 0, 0, 255), "%s v%u (%016llX)", fileName, cia.version, cia.titleID);
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
				wait(100000000);
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
				wait(100000000);
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