#include <3ds.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_main.h"
#include "C2D_helper.h"
#include "textures.h"
#include "utils.h"

static u32 cpu_time_limit = 0;

static void Term_Services(void) {
	Textures_Free();

	if (Utils_IsN3DS())
		osSetSpeedupEnable(false);

	FS_CloseArchive(nand_archive);
	FS_CloseArchive(sdmc_archive);

	C2D_FontFree(font);
	C2D_TextBufDelete(c2d_size_buf);
	C2D_TextBufDelete(c2d_dynamic_buf);
	C2D_TextBufDelete(c2d_static_buf);

	if (cpu_time_limit != UINT32_MAX)
		APT_SetAppCpuTimeLimit(cpu_time_limit);

	C2D_Fini();
	C3D_Fini();
	gfxExit();
	cfguExit();
	romfsExit();
	ptmuExit();
	mcuHwcExit();
	amExit();
	acExit();
}

static void Init_Services(void) {
	acInit();
	amInit();
	AM_QueryAvailableExternalTitleDatabase(NULL);
	mcuHwcInit();
	ptmuInit();
	romfsInit();
	cfguInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	if (Utils_IsN3DS())
		osSetSpeedupEnable(true);

	APT_GetAppCpuTimeLimit(&cpu_time_limit);
	APT_SetAppCpuTimeLimit(30);

	c2d_static_buf = C2D_TextBufNew(4096);
	c2d_dynamic_buf = C2D_TextBufNew(4096);
	c2d_size_buf = C2D_TextBufNew(4096);
	font = C2D_FontLoad("romfs:/res/drawable/Roboto-Regular.bcfnt");

	RENDER_TOP = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	RENDER_BOTTOM = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	FS_OpenArchive(&sdmc_archive, ARCHIVE_SDMC);
	FS_OpenArchive(&nand_archive, ARCHIVE_NAND_CTR_FS);
	archive = sdmc_archive;

	FS_RecursiveMakeDir(archive, "/3ds/3DShell/");
	
	Textures_Load();
	Config_Load();
	Config_GetLastDirectory();
}

int main(void) {
	Init_Services();

	if (setjmp(exitJmp)) {
		Term_Services();
		return 0;
	}

	MENU_STATE = MENU_STATE_HOME;
	BROWSE_STATE = BROWSE_STATE_SD;
	Menu_Main();
	Term_Services();

	return 0;
}
