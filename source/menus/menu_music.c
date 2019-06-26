#include <3ds.h>
#include <stdlib.h>

#include "3dsaudiolib.h"
#include "audio.h"
#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

#define MUSIC_GENRE_COLOUR      C2D_Color32(97, 97, 97, 255)
#define MUSIC_STATUS_BG_COLOUR  C2D_Color32(43, 53, 61, 255)
#define MUSIC_SEPARATOR_COLOUR  C2D_Color32(34, 41, 48, 255)

typedef enum {
	MUSIC_STATE_NONE,   // 0
	MUSIC_STATE_REPEAT, // 1
	MUSIC_STATE_SHUFFLE // 2
} Music_State;

static char playlist[1024][512];
static int count = 0, selection = 0, state = 0;
static float title_height = 0, length_time_width = 0;
char *position_time = NULL, *length_time = NULL, *filename = NULL;

static Result Menu_GetMusicList(void) {
	Handle dir;
	Result ret = 0;
	
	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_ASCII, cwd)))) {
		u32 entryCount = 0;
		FS_DirectoryEntry *entries = (FS_DirectoryEntry *)calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
		
		if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries))) {
			qsort(entries, entryCount, sizeof(FS_DirectoryEntry), Utils_Alphasort);
			char name[256] = {'\0'};

			for (u32 i = 0; i < entryCount; i++) {
				Utils_U16_To_U8((u8 *)&name[0], entries[i].name, 255);

				if (/*(!strncasecmp(entries[i].shortExt, "fla", 3)) || */(!strncasecmp(entries[i].shortExt, "it", 2))
					|| (!strncasecmp(entries[i].shortExt, "mod", 3)) || (!strncasecmp(entries[i].shortExt, "mp3", 3))
					|| (!strncasecmp(entries[i].shortExt, "ogg", 3)) /*|| (!strncasecmp(entries[i].shortExt, "opu", 3))*/
					|| (!strncasecmp(entries[i].shortExt, "s3m", 3)) || (!strncasecmp(entries[i].shortExt, "wav", 3))
					|| (!strncasecmp(entries[i].shortExt, "xm", 2))) {
					strcpy(playlist[count], cwd);
					strcpy(playlist[count] + strlen(playlist[count]), name);
					count++;
				}
			}
		}
		else {
			free(entries);
			return ret;
		}
		
		free(entries);

		if (R_FAILED(ret = FSDIR_Close(dir))) // Close directory
			return ret;
	}
	else
		return ret;

	return 0;
}

static int Music_GetCurrentIndex(char *path) {
	for(int i = 0; i < count; ++i) {
		if (!strcmp(playlist[i], path))
			return i;
	}

	return 0;
}

static void Menu_ConvertSecondsToString(char *string, u64 seconds) {
	int h = 0, m = 0, s = 0;
	h = (seconds / 3600);
	m = (seconds - (3600 * h)) / 60;
	s = (seconds - (3600 * h) - (m * 60));

	if (h > 0)
		snprintf(string, 35, "%02d:%02d:%02d", h, m, s);
	else
		snprintf(string, 35, "%02d:%02d", m, s);
}

static void Music_Play(char *path) {
	Audio_Init(path);

	filename = malloc(128);
	snprintf(filename, 128, Utils_Basename(path));
	position_time = malloc(35);
	length_time = malloc(35);
	length_time_width = 0;

	Menu_ConvertSecondsToString(length_time, Audio_GetLengthSeconds());
	length_time_width = Draw_GetTextWidth(0.45f, length_time);
	title_height = Draw_GetTextHeight(0.5f, strupr(filename));
	selection = Music_GetCurrentIndex(path);
}

static void Music_HandleNext(bool forward, int state) {
	if (state == MUSIC_STATE_NONE) {
		if (forward)
			selection++;
		else
			selection--;
	}
	else if (state == MUSIC_STATE_SHUFFLE) {
		int old_selection = selection;
		time_t t;
		srand((unsigned) time(&t));
		selection = rand() % (count - 1);

		if (selection == old_selection)
			selection++;
	}

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	Audio_Stop();

	free(filename);
	free(length_time);
	free(position_time);

	Audio_Term();
	Music_Play(playlist[selection]);
}

void Menu_PlayMusic(char *path) {
	aptSetSleepAllowed(false);
	Menu_GetMusicList();
	Music_Play(path);

	bool locked = false;
	
	while (aptMainLoop()) {
		_3dsAudioRunThread();
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, config.dark_theme? BLACK_BG : WHITE);
		C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_TOP);

		Draw_Image(default_artwork_blur, 0, 0);
		Draw_Rect(0, 0, 400, 18, MUSIC_GENRE_COLOUR); // Status bar
		Draw_Rect(0, 55, 400, 2, MUSIC_SEPARATOR_COLOUR); // Separating line

		if (locked)
			Draw_Image(icon_lock, 2, 2);

		if ((metadata.has_meta) && (metadata.title[0] != '\0') && (metadata.artist[0] != '\0')) {
			Draw_Text(5, 22, 0.5f, WHITE, strupr(metadata.title));
			Draw_Text(5, 38, 0.45f, WHITE, strupr(metadata.artist));
		}
		else if ((metadata.has_meta) && (metadata.title[0] != '\0'))
			Draw_Text(5, ((37 - title_height) / 2) + 18, 0.5f, WHITE, strupr(metadata.title));
		else
			Draw_Text(5, ((37 - title_height) / 2) + 18, 0.5f, WHITE, strupr(filename));

		StatusBar_DisplayTime();

		Draw_Rect(178, 57, 222, 175, C2D_Color32(45, 48, 50, 255));
		Draw_Rect(183, 62, 212, 165, C2D_Color32(46, 49, 51, 255));

		Draw_Rect(0, 57, 175, 175, MUSIC_GENRE_COLOUR);

		if (metadata.has_meta) {
			if (metadata.album[0] != '\0')
				Draw_Textf(185, 64, 0.5f, WHITE, "%.30s", metadata.album);

			if (metadata.year[0] != '\0')
				Draw_Textf(185, 84, 0.5f, WHITE, "%.30s", metadata.year);
			
			if (metadata.genre[0] != '\0')
				Draw_Textf(185, 104, 0.5f, WHITE, "%.30s", metadata.genre);
		}

		if (metadata.cover_image.tex)
			Draw_ImageScale(metadata.cover_image, 0, 57, (175.0f / metadata.cover_image.subtex->width), (175.0f / metadata.cover_image.subtex->height));
		else
			Draw_Image(default_artwork, 0, 57); // Default album art

		Menu_ConvertSecondsToString(position_time, Audio_GetPositionSeconds());
		Draw_Text(185, 205, 0.45f, WHITE, position_time);
		Draw_Text(392 - length_time_width, 205, 0.45f, WHITE, length_time);

		// Progress bar
		if (Audio_GetPosition() != -1) {
			Draw_Rect(185, 220, 207, 2, C2D_Color32(97, 97, 97, 150));
			Draw_Rect(185, 220, (((double)Audio_GetPosition()/(double)Audio_GetLength()) * 207.0), 2, WHITE);
		}

		C2D_SceneBegin(RENDER_BOTTOM);

		Draw_Image(ic_music_bg_bottom, 0, 0);

		if (!Audio_IsPaused())
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
			Audio_Pause();

		if ((kDown & KEY_Y) || ((TouchInRect(((320 - btn_repeat.subtex->width) / 2) + 65, ((240 - btn_shuffle.subtex->height) / 2) + 35, 
			(((320 - btn_repeat.subtex->width) / 2) + 65) + 30, (((240 - btn_shuffle.subtex->height) / 2) + 35) + 30)) && (kDown & KEY_TOUCH))) {
			if (state == MUSIC_STATE_REPEAT)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_REPEAT;
		}
		else if ((kDown & KEY_X) || ((TouchInRect(((320 - btn_shuffle.subtex->width) / 2) - 65, ((240 - btn_shuffle.subtex->height) / 2) + 35, 
			(((320 - btn_shuffle.subtex->width) / 2) - 65) + 30, (((240 - btn_shuffle.subtex->height) / 2) + 35) + 30)) && (kDown & KEY_TOUCH))) {
			if (state == MUSIC_STATE_SHUFFLE)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_SHUFFLE;
		}

		if (!locked) {
			if ((kDown & KEY_LEFT) || (kDown & KEY_L) || ((TouchInRect(((320 - btn_rewind.subtex->width) / 2) - 80, ((240 - btn_rewind.subtex->height) / 2), 
				(((320 - btn_rewind.subtex->width) / 2) - 80) + 45, ((240 - btn_rewind.subtex->height) / 2) + 45)) && (kDown & KEY_TOUCH))) {
				if (count != 0)
					Music_HandleNext(false, MUSIC_STATE_NONE);
			}
			else if ((kDown & KEY_RIGHT) || (kDown & KEY_R) || ((TouchInRect(((320 - btn_forward.subtex->width) / 2) + 80, ((240 - btn_forward.subtex->height) / 2), 
				(((320 - btn_forward.subtex->width) / 2) + 80) + 45, ((240 - btn_forward.subtex->height) / 2) + 45)) && (kDown & KEY_TOUCH))) {
				if (count != 0)
					Music_HandleNext(true, MUSIC_STATE_NONE);
			}
			if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
				Screenshot_Capture();
		}

		if (kDown & KEY_B) {
			Audio_Stop();
			break;
		}

		if (!playing) {
			if (state == MUSIC_STATE_NONE) {
				if (count != 0)
					Music_HandleNext(true, MUSIC_STATE_NONE);
			}
			else if (state == MUSIC_STATE_REPEAT)
				Music_HandleNext(false, MUSIC_STATE_REPEAT);
			else if (state == MUSIC_STATE_SHUFFLE) {
				if (count != 0)
					Music_HandleNext(false, MUSIC_STATE_SHUFFLE);
			}
		}
	}

	free(filename);
	free(length_time);
	free(position_time);

	Audio_Term();

	memset(playlist, 0, sizeof(playlist[0][0]) * 512 * 512);
	count = 0;
	aptSetSleepAllowed(true);
}
