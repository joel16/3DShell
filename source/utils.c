#include "unzip/archive.h"

#include "common.h"
#include "fs.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void installDirectories()
{
	if (BROWSE_STATE != STATE_NAND)
	{
		if (!(dirExists(fsArchive, "/3ds/")))
			makeDir(fsArchive, "/3ds");
		if (!(dirExists(fsArchive, "/3ds/3DShell/")))
			makeDir(fsArchive, "/3ds/3DShell");
		if (!(dirExists(fsArchive, "/3ds/3DShell/themes/")))
			makeDir(fsArchive, "/3ds/3DShell/themes");
		if (!(dirExists(fsArchive, "/3ds/3DShell/themes/default/")))
			makeDir(fsArchive, "/3ds/3DShell/themes/default");
		if (!(dirExists(fsArchive, "/3ds/3DShell/colours/")))
			makeDir(fsArchive, "/3ds/3DShell/colours");
	
		if (fileExists(fsArchive, "/3ds/3DShell/lastdir.txt"))
		{
			char buf[250];
		
			FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
			fscanf(read, "%s", buf);
			fclose(read);
		
			if (dirExists(fsArchive, buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
				strcpy(cwd, buf);
			else 
				strcpy(cwd, START_PATH);
		}
		else
		{
			writeFile("/3ds/3DShell/lastdir.txt", START_PATH);
			strcpy(cwd, START_PATH); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
		}
	
		/*if (!fileExists(fsArchive, "/3ds/3DShell/bgm.txt"))
		{
			setBgm(true);
		}
		else
		{
			int initBgm = 0;
		
			FILE * read = fopen("/3ds/3DShell/bgm.txt", "r");
			fscanf(read, "%d", &initBgm);
			fclose(read);
		
			if (initBgm == 0)
				bgmEnable = false;
			else 
				bgmEnable = true;
		}*/
	
		if (!fileExists(fsArchive, "/3ds/3DShell/sysProtection.txt")) // Initially set it to true
		{
			setConfig("/3ds/3DShell/sysProtection.txt", true);
			sysProtection = true;
		}
		else
		{
			int info = 0;
		
			FILE * read = fopen("/3ds/3DShell/sysProtection.txt", "r");
			fscanf(read, "%d", &info);
			fclose(read);
		
			if (info == 0)
				sysProtection = false;
			else 
				sysProtection = true;
		}
	}
}

void setBilinearFilter(sf2d_texture * texture)
{
	sf2d_texture_set_params(texture, GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_NEAREST));
}

void endDrawing()
{
	sf2d_end_frame();
	sf2d_swapbuffers();
}

void getSizeString(char * string, uint64_t size) //Thanks TheOfficialFloW
{
	double double_size = (double)size;

	int i = 0;
	static char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (double_size >= 1024.0f) {
		double_size /= 1024.0f;
		i++;
	}

	sprintf(string, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

int touchGetX()
{
	touchPosition pos;	
	hidTouchRead(&pos);
	return pos.px;
}

int touchGetY()
{
	touchPosition pos;	
	hidTouchRead(&pos);
	return pos.py;
}

sf2d_texture * sfil_load_IMG_file(const char * filename, sf2d_place place)
{
	int w, h;
	unsigned char *data = stbi_load(filename, &w, &h, NULL, 4);
	
	if (data == NULL) 
		return NULL;
	
	sf2d_texture * texture = NULL;
	texture = sf2d_create_texture_mem_RGBA8(data, w, h, TEXFMT_RGBA8, place);
	stbi_image_free(data);
	
	return texture;
}

int extractZip(const char * zipFile, const char * path) 
{
	char tmpFile2[1024];
	char tmpPath2[1024];
	FS_Archive fsArchive = 0;
	
	FSUSER_OpenArchive(&fsArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	FS_Path tempPath = fsMakePath(PATH_ASCII, path);
	FSUSER_CreateDirectory(fsArchive, tempPath, FS_ATTRIBUTE_DIRECTORY);
	FSUSER_CloseArchive(fsArchive);
	
	strcpy(tmpPath2, "sdmc:");
	strcat(tmpPath2, (char *)path);
	chdir(tmpPath2);
	
	if (strncmp("romfs:/", zipFile, 7) == 0) 
		strcpy(tmpFile2, zipFile);
	else
	{
		strcpy(tmpFile2, "sdmc:");
		strcat(tmpFile2, (char*)zipFile);
	}
	
	Zip *handle = ZipOpen(tmpFile2);
	
	if (handle == NULL) 
		return 0;
	
	int result = ZipExtract(handle, NULL);
	ZipClose(handle);
	return result;
}

void setConfig(const char * path, bool set) // using individual txt files for configs for now (plan to change this later when there's more options)
{
	if (set == true)
		writeFile(path, "1");
	else
		writeFile(path, "0");
}

const char * getLastNChars(char * str, int n)
{
	int len = strlen(str);
	const char *last_n = &str[len - n];
	return last_n;
}

u8 getRegion()
{
	u8 region;
	CFGU_SecureInfoGetRegion(&region);
	
	return region;
}

u8 getLanguage()
{
	u8 language;
	CFGU_GetSystemLanguage(&language);
	
	return language;
}

const char * getUsername() 
{
	int i;
	size_t size = 0x16;
	u8 * temp = (u8*)malloc(size);
	char * username = (char*)malloc(size / 2);
	
	for(i = 0; i < (size / 2); i++)
		username[i] = 0;
	
	CFGU_GetConfigInfoBlk2(0x1C, 0xA0000, temp);
	
	for(i = 0; i < (size / 2); i++)
		username[i] = (char)((u16*)temp)[i];
	
	return username;
}

bool isN3DS()
{
	bool isNew3DS = 0;
	APT_CheckNew3DS(&isNew3DS);
    
	if (isNew3DS)
		return true;
	else
		return false;
}

void utf2ascii(char* dst, u16* src)
{
	if(!src || !dst)
		return;
	
	while(*src)*(dst++)=(*(src++))&0xFF;
	*dst=0x00;
}

void utfn2ascii(char* dst, u16* src, int max)
{
	if(!src || !dst)return;
	int n=0;
	while(*src && n<max-1){*(dst++)=(*(src++))&0xFF;n++;}
	*dst=0x00;
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