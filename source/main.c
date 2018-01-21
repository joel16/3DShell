#include <3ds.h>

#include "common.h"
#include "fs.h"
#include "language.h"
#include "menus/menu_main.h"
#include "pp2d/pp2d.h"
#include "textures.h"
#include "utils.h"

static void Init_Services(void)
{
	acInit();
	amInit();
	cfguInit();
	//httpcInit(0);
	mcuHwcInit();
	ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ptmuInit();
	pp2d_init();
	romfsInit();

	FS_OpenArchive(&archive, ARCHIVE_SDMC);

	if (Utils_IsN3DS())
		osSetSpeedupEnable(true);

	APT_SetAppCpuTimeLimit(30);

	Load_Textures();

	language = 1; //Utils_GetLanguage();

	Utils_MakeDirectories();
	Utils_LoadConfig();
	Utils_GetLastDirectory();

	BROWSE_STATE = STATE_SD;
	MENU_DEFAULT_STATE = MENU_STATE_HOME;
}

static void Term_Services(void)
{
	Free_Textures();

	if (Utils_IsN3DS())
		osSetSpeedupEnable(false);

	FS_CloseArchive(archive);
	
	romfsExit();
	pp2d_exit();
	ptmuExit();
	ndspExit();
	mcuHwcExit();
	//httpcExit();
	cfguExit();
	amExit();
	acExit();
}

int main(int argc, char *argv[])
{
	Init_Services();
	Menu_Main();
	Term_Services();
}
