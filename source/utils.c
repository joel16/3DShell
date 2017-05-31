#include "unzip/archive.h"

#include "main.h"
#include "utils.h"

void setBilinearFilter(sf2d_texture *texture)
{
	sf2d_texture_set_params(texture, GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_NEAREST));
}

void endDrawing()
{
	sf2d_end_frame();
	sf2d_swapbuffers();
}

void getSizeString(char *string, uint64_t size) //Thanks TheOfficialFloW
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

int extractZip(const char * zipFile, const char * path) 
{
	char tmpFile2[1024];
	char tmpPath2[1024];
	FS_Archive sdmcArchive = 0;
	
	FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	FS_Path tempPath = fsMakePath(PATH_ASCII, path);
	FSUSER_CreateDirectory(sdmcArchive, tempPath, FS_ATTRIBUTE_DIRECTORY);
	FSUSER_CloseArchive(sdmcArchive);
	
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

void setConfig(char * path, bool set) //using individual txt files for configs for now (plan to change this later when there's more options)
{
	if (set == true)
	{
		int config = 1;
		FILE * write = fopen(path, "w");
		fprintf(write, "%d", config);
		fclose(write);
	}
	else
	{
		int config = 0;
		FILE * write = fopen(path, "w");
		fprintf(write, "%d", config);
		fclose(write);
	}
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

bool isN3DS()
{
	bool isNew3DS = 0;
    APT_CheckNew3DS(&isNew3DS);
    
	if (isNew3DS)
		return true;
	else
		return false;
}