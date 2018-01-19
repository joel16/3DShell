#include <3ds.h>

#include "common.h"
#include "dir_list.h"
#include "menu_main.h"
#include "menu_ftp.h"
#include "pp2d.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"

static void Menu_Main_Controls()
{
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
		Screenshot_Capture();

	if (kDown & KEY_SELECT)
		Menu_DisplayFTP();

	if (fileCount > 0)
	{	
		// Position Decrement
		if (kDown & KEY_DUP)
		{
			// Decrease Position
			if (position > 0)
				position--;

			// Rewind Pointer
			else 
				position = fileCount - 1;
			
			// Display file list
			Dirlist_DisplayFiles();
		}

		// Position Increment
		else if (kDown & KEY_DDOWN)
		{
			// Increase Position
			if (position < (fileCount - 1))
				position++;

			// Rewind Pointer
			else 
				position = 0;

			// Display file list
			Dirlist_DisplayFiles();
		}

		if (kHeld & KEY_CPAD_UP)
		{
			wait(5);

			//scroll_x = 395;
			//scroll_time = osGetTime();

			if (position > 0)
				position--;

			else position = fileCount - 1;
				Dirlist_DisplayFiles();
		}

		else if (kHeld & KEY_CPAD_DOWN)
		{
			wait(5);

			//scroll_x = 395;
			//scroll_time = osGetTime();

			if (position < (fileCount - 1))
				position++;

			else position = 0;

			Dirlist_DisplayFiles();
		}

		else if (kDown & KEY_A)
		{
			wait(1);
			Dirlist_OpenFile(); // Open file/dir
		}

		else if ((strcmp(cwd, ROOT_PATH) != 0) && (kDown & KEY_B))
		{
			wait(1);
			Dirlist_Navigate(false);
			Dirlist_PopulateFiles(true);
			Dirlist_DisplayFiles();
		}
	}
}

void Menu_Main(void)
{
	Dirlist_PopulateFiles(true);

	while (aptMainLoop())
	{
		pp2d_begin_draw(GFX_TOP, GFX_LEFT);
			pp2d_draw_texture(TEXTURE_BACKGROUND, 0, 0);
			StatusBar_DisplayBar();
			Dirlist_DisplayFiles();
		pp2d_end_draw();

		pp2d_begin_draw(GFX_BOTTOM, GFX_LEFT);
			pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(30, 136, 229, 255));
			pp2d_draw_rectangle(0, 0, 320, 20, RGBA8(25, 118, 210, 255));
		pp2d_end_draw();

		hidScanInput();
		Menu_Main_Controls();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
	}
}