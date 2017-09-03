#include "cia.h"
#include "common.h"
#include "file/dirlist.h"
#include "file/fs.h"
#include "main.h"
#include "net/net.h"
#include "graphics/screen.h"
#include "net/updater.h"
#include "utils.h"

char * checkForUpdate(void)
{
	int currVer = 0, updateVer = 0;
	static char str[20];

	Result ret = downloadFile("https://github.com/3DShell-Bot/Updater/releases/download/UPDATER/version.txt", "/3ds/3DShell/version.txt");

	currVer = VERSION_MAJOR + VERSION_MINOR; // A shitty way to get updates I know - I plan to change this later.

	if (R_SUCCEEDED(ret))
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

	if (R_SUCCEEDED(ret))
	{
		screen_draw_string(((320 - screen_get_string_width("Downloaded update", 0.41f, 0.41f)) / 2), 60, 0.41f, 0.41f, RGBA8(251, 251, 251, 255), "Downloaded update");
		wait(100000000);
		installUpdate();
	}
}

void installUpdate(void)
{
	screen_draw_string(((320 - screen_get_string_width("Update completed", 0.41f, 0.41f)) / 2), 80, 0.41f, 0.41f, RGBA8(251, 251, 251, 255), "Update completed");

	//installCIA("/3ds/3DShell/3DShell.cia");

	wait(200000000);

	mainMenu(CLEAR);
}
