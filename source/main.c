#include <stdlib.h>
#include <3ds.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_main.h"
#include "C2D_helper.h"
#include "textures.h"
#include "utils.h"

static void Term_Services(void)
{
	Textures_Free();

	if (Utils_IsN3DS())
		osSetSpeedupEnable(false);

	FS_CloseArchive(archive);

	C2D_TextBufDelete(sizeBuf);
	C2D_TextBufDelete(dynamicBuf);
	C2D_TextBufDelete(staticBuf);

	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	ptmuExit();
	ndspExit();
	mcuHwcExit();
	cfguExit();
	amExit();
	acExit();
}

static void Init_Services(void)
{
	acInit();
	amInit();
	AM_QueryAvailableExternalTitleDatabase(NULL);
	cfguInit();
	mcuHwcInit();
	ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ptmuInit();
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	if (Utils_IsN3DS())
		osSetSpeedupEnable(true);

	APT_SetAppCpuTimeLimit(30);

	staticBuf = C2D_TextBufNew(4096);
	dynamicBuf = C2D_TextBufNew(4096);
	sizeBuf = C2D_TextBufNew(4096);

	RENDER_TOP = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    RENDER_BOTTOM = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	FS_OpenArchive(&archive, ARCHIVE_SDMC);

	FS_RecursiveMakeDir(archive, "/3ds/3DShell/");
	
	Textures_Load();
	Config_Load();
	Config_GetLastDirectory();
}

int main(int argc, char *argv[])
{
	Init_Services();
	//Config_Load();

	if (setjmp(exitJmp)) 
	{
		Term_Services();
		return 0;
	}

	MENU_DEFAULT_STATE = MENU_STATE_HOME;
	Menu_Main();
	Term_Services();

	return 0;
}
