#include <time.h>

#include <3ds.h>

#include "audio.h"
#include "mp3.h"

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_music.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

#define MUSIC_GENRE_COLOUR      C2D_Color32(97, 97, 97, 255)
#define MUSIC_STATUS_BG_COLOUR  C2D_Color32(43, 53, 61, 255)
#define MUSIC_SEPARATOR_COLOUR  C2D_Color32(34, 41, 48, 255)

typedef enum
{
	MUSIC_STATE_NONE,   // 0
	MUSIC_STATE_REPEAT, // 1
	MUSIC_STATE_SHUFFLE // 2
} Music_State;

static Thread thread = NULL;
static bool isMP3 = false;
static char playlist[512][512], title[128];
static int count = 0, selection = 0, state = 0;

static Result Menu_GetMusicList(void)
{
	Handle dir;
	Result ret = 0;
	
	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_ASCII, cwd))))
	{
		u32 entryCount = 0;
		FS_DirectoryEntry* entries = (FS_DirectoryEntry*) calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
		
		if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries)))
		{
			qsort(entries, entryCount, sizeof(FS_DirectoryEntry), Utils_Alphasort);
			u8 name[256] = {'\0'};

			for (u32 i = 0; i < entryCount; i++) 
			{
				Utils_U16_To_U8(&name[0], entries[i].name, 255);
				int length = strlen(name);

				if ((strncasecmp(entries[i].shortExt, "mp3", 3) == 0) || (strncasecmp(entries[i].shortExt, "ogg", 3) == 0) 
					|| (strncasecmp(entries[i].shortExt, "fla", 3) == 0) || (strncasecmp(entries[i].shortExt, "wav", 3) == 0))
				{
					strcpy(playlist[count], cwd);
					strcpy(playlist[count] + strlen(playlist[count]), name);
					count++;
				}
			}
		}
		else
		{
			free(entries);
			return ret;
		}
		
		free(entries);

		if (R_FAILED(ret = FSDIR_Close(dir))) // Close directory
			return ret;
	}
	else
		return ret;
}

static int Music_GetCurrentIndex(char *path)
{
	for(int i = 0; i < count; ++i)
	{
		if (!strcmp(playlist[i], path))
			return i;
	}
}

static void Music_Play(char *path)
{
	Menu_GetMusicList();

	/* Reset previous stop command */
	stop = false;

	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	thread = threadCreate(Audio_PlayFile, path, 32 *1024, prio - 1, -2, false);

	selection = Music_GetCurrentIndex(path);
	strncpy(title, strlen(ID3.title) == 0? strupr(Utils_Basename(path)) : strupr(ID3.title), strlen(ID3.title) == 0? strlen(Utils_Basename(path)) + 1 : strlen(ID3.title) + 1);

	isMP3 = (strncasecmp(&path[strlen(path)-3], "mp3", 3) == 0);
}


static void Music_HandleNext(bool forward, int state)
{
	if (state == MUSIC_STATE_NONE)
	{
		if (forward)
			selection++;
		else
			selection--;
	}
	else if (state == MUSIC_STATE_SHUFFLE)
	{
		int old_selection = selection;
		time_t t;
		srand((unsigned) time(&t));
		selection = rand() % (count - 1);

		if (selection == old_selection)
			selection++;
	}

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	wait(1);
	Audio_StopPlayback();
	memset(title, 0, sizeof(title));

	memset(ID3.artist, 0, 30);
	memset(ID3.title, 0, 30);
	memset(ID3.album, 0, 30);
	memset(ID3.year, 0, 4);
	memset(ID3.genre, 0, 30);

	threadJoin(thread, U64_MAX);
	threadFree(thread);

	Music_Play(playlist[selection]);
}

void Menu_PlayMusic(char *path)
{
	aptSetSleepAllowed(false);
	Music_Play(path);

	bool locked = false;
	
	while (aptMainLoop())
	{
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, config_dark_theme? BLACK_BG : WHITE);
		C2D_TargetClear(RENDER_BOTTOM, config_dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_TOP);

		Draw_Image(default_artwork_blur, 0, 0);
		Draw_Rect(0, 0, 400, 18, MUSIC_GENRE_COLOUR); // Status bar
		Draw_Rect(0, 55, 400, 2, MUSIC_SEPARATOR_COLOUR); // Separating line

		if (locked)
			Draw_Image(icon_lock, 2, 2);

		StatusBar_DisplayTime();

		Draw_Rect(178, 57, 222, 175, C2D_Color32(45, 48, 50, 255));
		Draw_Rect(183, 62, 212, 165, C2D_Color32(46, 49, 51, 255));

		if (isMP3) // Only print out ID3 tag info for MP3
		{	
			Draw_Text(5, 22, 0.5f, WHITE, strupr(title));
			Draw_Text(5, 38, 0.45f, WHITE, strupr(ID3.artist));

			Draw_Textf(184, 64, 0.5f, WHITE, "%.30s", ID3.album);
			Draw_Textf(184, 84, 0.5f, WHITE, "%.30s", ID3.year);
			Draw_Textf(184, 104, 0.5f, WHITE, "%.30s", ID3.genre);
		}
		else
			Draw_Text(5, ((37 - Draw_GetTextHeight(0.5f, title)) / 2) + 18, 0.5f, WHITE, title);

		Draw_Rect(0, 57, 175, 175, MUSIC_GENRE_COLOUR);
		Draw_Image(default_artwork, 0, 57);

		C2D_SceneBegin(RENDER_BOTTOM);

		Draw_Image(ic_music_bg_bottom, 0, 0);

		if (!(Audio_IsPaused(SFX)))
			Draw_Image(btn_pause, ((320 - btn_pause.subtex->width) / 2) - 2, ((240 - btn_pause.subtex->height) / 2));
		else
			Draw_Image(btn_play, ((320 - btn_play.subtex->width) / 2), ((240 - btn_play.subtex->height) / 2));

		Draw_Image(btn_rewind, ((320 - btn_rewind.subtex->width) / 2) - 80, ((240 - btn_rewind.subtex->height) / 2));
		Draw_Image(btn_forward, ((320 - btn_forward.subtex->width) / 2) + 80, ((240 - btn_forward.subtex->height) / 2));

		Draw_Image(state == MUSIC_STATE_SHUFFLE? btn_shuffle_overlay : btn_shuffle, ((320 - btn_shuffle.subtex->width) / 2) - 65, ((240 - btn_shuffle.subtex->height) / 2) + 35);
		Draw_Image(state == MUSIC_STATE_REPEAT? btn_repeat_overlay : btn_repeat, ((320 - btn_repeat.subtex->width) / 2) + 65, ((240 - btn_repeat.subtex->height) / 2) + 35);
		
		Draw_EndFrame();

		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		if (kDown & KEY_START)
			locked = !locked;

		if ((kDown & KEY_A) || ((TouchInRect(114, 76, 204, 164)) && (kDown & KEY_TOUCH)))
			Audio_TogglePlayback(SFX);

		if ((kDown & KEY_Y) || ((TouchInRect(((320 - btn_repeat.subtex->width) / 2) + 65, ((240 - btn_shuffle.subtex->height) / 2) + 35, 
			(((320 - btn_repeat.subtex->width) / 2) + 65) + 30, (((240 - btn_shuffle.subtex->height) / 2) + 35) + 30)) && (kDown & KEY_TOUCH)))
		{
			if (state == MUSIC_STATE_REPEAT)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_REPEAT;
		}
		else if ((kDown & KEY_X) || ((TouchInRect(((320 - btn_shuffle.subtex->width) / 2) - 65, ((240 - btn_shuffle.subtex->height) / 2) + 35, 
			(((320 - btn_shuffle.subtex->width) / 2) - 65) + 30, (((240 - btn_shuffle.subtex->height) / 2) + 35) + 30)) && (kDown & KEY_TOUCH)))
		{
			if (state == MUSIC_STATE_SHUFFLE)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_SHUFFLE;
		}

		if (!locked)
		{
			if ((kDown & KEY_LEFT) || (kDown & KEY_L) || ((TouchInRect(((320 - btn_rewind.subtex->width) / 2) - 80, ((240 - btn_rewind.subtex->height) / 2), 
				(((320 - btn_rewind.subtex->width) / 2) - 80) + 45, ((240 - btn_rewind.subtex->height) / 2) + 45)) && (kDown & KEY_TOUCH)))
			{
				wait(1);
				
				if (count != 0)
					Music_HandleNext(false, MUSIC_STATE_NONE);
			}
			else if ((kDown & KEY_RIGHT) || (kDown & KEY_R) || ((TouchInRect(((320 - btn_forward.subtex->width) / 2) + 80, ((240 - btn_forward.subtex->height) / 2), 
				(((320 - btn_forward.subtex->width) / 2) + 80) + 45, ((240 - btn_forward.subtex->height) / 2) + 45)) && (kDown & KEY_TOUCH)))
			{
				wait(1);
				
				if (count != 0)
					Music_HandleNext(true, MUSIC_STATE_NONE);
			}
			if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
				Screenshot_Capture();
		}

		if (kDown & KEY_B)
		{
			wait(1);
			Audio_StopPlayback();
			break;
		}

		if (!Audio_IsPlaying())
		{
			wait(1);

			if (state == MUSIC_STATE_NONE)
			{
				if (count != 0)
					Music_HandleNext(true, MUSIC_STATE_NONE);
			}
			else if (state == MUSIC_STATE_REPEAT)
				Music_HandleNext(false, MUSIC_STATE_REPEAT);
			else if (state == MUSIC_STATE_SHUFFLE)
			{
				if (count != 0)
					Music_HandleNext(false, MUSIC_STATE_SHUFFLE);
			}
		}
	}

	threadJoin(thread, U64_MAX);
	threadFree(thread);
	
	// Clear ID3
	if (isMP3)
	{
		memset(ID3.artist, 0, 30);
		memset(ID3.title, 0, 30);
		memset(ID3.album, 0, 30);
		memset(ID3.year, 0, 4);
		memset(ID3.genre, 0, 30);
	}

	memset(title, 0, sizeof(title));
	memset(playlist, 0, sizeof(playlist[0][0]) * 512 * 512);
	count = 0;
	aptSetSleepAllowed(true);
	return;
}