#include "common.h"
#include "fs.h"
#include "utils.h"

void installDirectories(void)
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
		
		if (!fileExists(fsArchive, "/3ds/3DShell/isHidden.txt")) // Initially set it to true
		{
			setConfig("/3ds/3DShell/isHidden.txt", true);
			isHiddenEnabled = true;
		}
		else
		{
			int info = 0;
		
			FILE * read = fopen("/3ds/3DShell/isHidden.txt", "r");
			fscanf(read, "%d", &info);
			fclose(read);
		
			if (info == 0)
				isHiddenEnabled = false;
			else 
				isHiddenEnabled = true;
		}
	}
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

int touchGetX(void)
{
	touchPosition pos;	
	hidTouchRead(&pos);
	return pos.px;
}

int touchGetY(void)
{
	touchPosition pos;	
	hidTouchRead(&pos);
	return pos.py;
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

u8 getRegion(void)
{
	u8 region;
	CFGU_SecureInfoGetRegion(&region);
	
	return region;
}

u8 getLanguage(void)
{
	u8 language;
	CFGU_GetSystemLanguage(&language);
	
	return language;
}

const char * getUsername(void) 
{
	u8 * data = (u8*)malloc(28);
	char * username = (char*)malloc(0x13);
    
	CFGU_GetConfigInfoBlk2(0x1C, 0x000A0000, data);

	for (int i = 0; i < 0x13; i++)
		username[i] = (char)((u16*)data)[i];
	
	return username;
}

bool isN3DS(void)
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