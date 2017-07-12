#include "cia.h"
#include "common.h"
#include "dirlist.h"
#include "fs.h"
#include "main.h"
#include "net.h"
#include "updater.h"
#include "utils.h"

char * checkForUpdate(void)
{
	int currVer = 0, updateVer = 0;
	static char str[20];
	
	Result ret = downloadFile("https://github.com/3DShell-Bot/Updater/releases/download/UPDATER/version.txt", "/3ds/3DShell/version.txt");
	
	currVer = VERSION_MAJOR + VERSION_MINOR; // A shitty way to get updates I know - I plan to change this later.
	
	if (ret == 0)
	{
		FILE * file = fopen("/3ds/3DShell/version.txt", "r");
		fscanf(file, "%d", &updateVer);
		fclose(file);
	}
	
	if (updateVer > currVer)
		strcpy(str, "Update found");
	else
		strcpy(str, "No updates found.");
	
	return str;
}

void downloadUpdate(void)
{
	Result ret = downloadFile("https://github.com/3DShell-Bot/Updater/releases/download/UPDATER/3DShell.cia", "/3ds/3DShell/3DShell.cia");
	
	if (ret == 0)
	{
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Downloaded update")) / 2), 60, RGBA8(251, 251, 251, 255), 11, "Downloaded update");
		wait(100000000);
		installUpdate();
	}
}

void installUpdate(void)
{	
	sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Update completed")) / 2), 80, RGBA8(251, 251, 251, 255), 11, "Update completed");
	
	//installCIA("/3ds/3DShell/3DShell.cia");
	
	wait(200000000);
	
	mainMenu(CLEAR);
}