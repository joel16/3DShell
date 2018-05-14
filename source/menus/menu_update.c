#include "cia.h"
#include "common.h"
#include "fs.h"
#include "language.h"
#include "menu_update.h"
#include "net.h"
#include "pp2d.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"
#include "utils.h"

struct colour Settings_colour;
struct colour Settings_title_text_colour;
struct colour Settings_text_colour;
struct colour Settings_text_min_colour;

struct colour BottomScreen_colour;
struct colour Options_title_text_colour;

static bool err = false;
char ver[10];

bool Menu_ValidateUpdate(bool nighlty)
{
	if (nighlty)
	{
		if (FS_FileExists(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt"))
		{
			u64 size64 = 0;
			u32 size = 0;

			size64 = FS_GetFileSize(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt");
			size = (u32)size64;
			char * buf = (char *)malloc(size + 1);

			if (R_FAILED(FS_Read(archive, "/3ds/3DShell/UPDATE_NIGHTLY.txt", size, buf)))
			{
				free(buf);
				return false;
			}

			buf[size] = '\0';
			sscanf(buf, "%s", ver);
			free(buf);

			if (strcmp(ver, GITVERSION) != 0)	
				return true;

			return false;
		}
	}
}

void Menu_DisplayUpdate(void)
{
	float err_width = ((320 - pp2d_get_text_height("No updates available.", 0.45f, 0.45f)) / 2);

	pp2d_draw_rectangle(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));

	pp2d_draw_text(10, 30, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "Update Center");

	if (err)
		pp2d_draw_text(err_width, 60, 0.45f, 0.45f,  RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "No updates available.");

	pp2d_draw_textf(122, 85, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Nightly builds");
	pp2d_draw_textf(52, 97, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Untested builds that may contain bugs.");

	pp2d_draw_rectangle(106, 117, (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 10), 20, 
		RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255));
	pp2d_draw_rectangle(110 - 3, 118, (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 8), 18, 
		RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
	pp2d_draw_rectangle(110 - 2, 119, (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 6), 16, 
		RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255));
	pp2d_draw_text(110, 120, 0.45f, 0.45f, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255), "Check for updates");

	pp2d_draw_textf(115, 155, 0.45f, 0.45f,  RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), "Milestone builds");
	pp2d_draw_textf(102, 167, 0.45f, 0.45f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "Official release builds.");
	
	pp2d_draw_rectangle(106, 187, (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 10), 20, 
		RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255));
	pp2d_draw_rectangle(110 - 3, 188, (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 8), 18, 
		RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
	pp2d_draw_rectangle(110 - 2, 189, (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 6), 16, 
		RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255));
	pp2d_draw_text(110, 190, 0.45f, 0.45f, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255), "Check for updates");
}

void Menu_DisplayUpdate2(void)
{
	pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));

	pp2d_draw_texture(TEXTURE_DIALOG, 20, 55);

	pp2d_draw_text(27, 72, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "Confirm update");

	pp2d_draw_text(206, 159, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "NO");
	pp2d_draw_text(255, 159, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "YES");

	pp2d_draw_text(((320 - pp2d_get_text_width(lang_deletion[language][1], 0.45f, 0.45f)) / 2), 100, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), "This action cannot be undone.");
	pp2d_draw_text(((320 - pp2d_get_text_width(lang_deletion[language][2], 0.45f, 0.45f)) / 2), 115, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), "Do you wish to update?");
}

void Menu_ControlUpdate(u32 input)
{
	if ((input & KEY_TOUCH) && (touchInRect(106, 117, (106) + (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 10), 137)))
	{
		wait(1);
		Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/UPDATE_NIGHTLY.txt", "/3ds/3DShell/UPDATE_NIGHTLY.txt");
		if (Menu_ValidateUpdate(true))
			MENU_DEFAULT_STATE = MENU_STATE_UPDATE_2;
		else
			err = true;
	}

	else if ((input & KEY_TOUCH) && (touchInRect(106, 187, (106) + (pp2d_get_text_width("Check for updates", 0.45f, 0.45f) + 10), 207)))
	{
		wait(1);
		Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/UPDATE_MILESTONE.txt", "/3ds/3DShell/UPDATE_MILESTONE.txt");
		if (Menu_ValidateUpdate(false))
			MENU_DEFAULT_STATE = MENU_STATE_UPDATE_2;
		else
			err = true;
	}
}

void Menu_ControlUpdate2(u32 input)
{
	if ((input & KEY_A) || (touchInRect(240, 142, 320, 185)))
	{	
		if(envIsHomebrew())
		{
			if (FS_FileExists(archive, "/3ds/3DShell/3DShell.3dsx"))
				FS_Remove(archive, "/3ds/3DShell/3DShell.3dsx");
			Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/3DShell.3dsx", "/3ds/3DShell/3DShell.3dsx");
			longjmp(exitJmp, 1);
		}
		else
		{
			Net_DownloadFile("https://github.com/joel16/3DShell/raw/gh-pages/3DShell.cia", "/3ds/3DShell/3DShell.cia");
			CIA_InstallTitle("/3ds/3DShell/3DShell.cia", MEDIATYPE_SD, true);
		}

		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_HOME;
	}

	else if ((input & KEY_B) || (touchInRect(136, 142, 239, 185)))
	{
		wait(1);
		MENU_DEFAULT_STATE = MENU_STATE_UPDATE;
	}
}