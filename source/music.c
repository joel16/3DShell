#include <3ds.h>
#include <stdio.h>

#include "audio/flac.h"
#include "audio/mp3.h"
#include "audio/ogg.h"
#include "audio/opus.h"
#include "audio/vorbis.h"
#include "clock.h"
#include "common.h"
#include "graphics/screen.h"
#include "music.h"
#include "power.h"
#include "screenshot.h"
#include "utils.h"
#include "wifi.h"

static struct audio * music;

/**
 * Obtains audio file type. Lifted from ctrmus with permission.
 *
 * \param	file	File location.
 * \return			file_types enum or 0 on unsupported file or error.
 */
static enum file_types getMusicFileType(const char *file)
{
	FILE* ftest = fopen(file, "rb");
	uint32_t fileSig;
	enum file_types file_type = FILE_TYPE_ERROR;

	/* Failure opening file */
	if(ftest == NULL)
		return -1;

	if(fread(&fileSig, 4, 1, ftest) == 0)
		goto err;

	switch(fileSig)
	{
		// "RIFF"
		case 0x46464952:
			if(fseek(ftest, 4, SEEK_CUR) != 0)
				break;

			// "WAVE"
			// Check required as AVI file format also uses "RIFF".
			if(fread(&fileSig, 4, 1, ftest) == 0)
				break;

			if(fileSig != 0x45564157)
				break;

			file_type = FILE_TYPE_WAV;
			break;

		// "fLaC"
		case 0x43614c66:
			file_type = FILE_TYPE_FLAC;
			break;

		// "OggS"
		case 0x5367674F:
			if(isOpus(file) == 0)
				file_type = FILE_TYPE_OPUS;
			else if(isFlac(file) == 0)
				file_type = FILE_TYPE_FLAC;
			else if(isVorbis(file) == 0)
				file_type = FILE_TYPE_VORBIS;

			break;

		default:
			/*
			 * MP3 without ID3 tag, ID3v1 tag is at the end of file, or MP3
			 * with ID3 tag at the beginning  of the file.
			 */
			if((fileSig << 16) == 0xFBFF0000 ||
					(fileSig << 16) == 0xFAFF0000 ||
					(fileSig << 8) == 0x33444900)
				file_type = FILE_TYPE_MP3;

			break;
	}

err:
	fclose(ftest);
	return file_type;
}

/**
 * Play an audio file.
 *
 * \param path	File path.
 */
void musicPlayer(char * path)
{
	enum file_types file_type = getMusicFileType(path);

	/* Unsupported file */
	if(file_type == FILE_TYPE_ERROR)
		return;

	//TODO
	music = mp3_create(SFX);

	if (music != NULL)
		mp3_load(path, music);
	else
		music->status = -1;

	while (aptMainLoop())
	{
		hidScanInput();

		screen_begin_frame();
		screen_select(GFX_BOTTOM);
		screen_draw_texture(TEXTURE_MUSIC_BOTTOM_BG, 0, 0);

		if (!(audio_isPaused(music)))
		{
			screen_draw_texture(TEXTURE_MUSIC_PAUSE, ((320 - screen_get_texture_width(TEXTURE_MUSIC_PAUSE)) / 2) - 2, ((240 - screen_get_texture_height(TEXTURE_MUSIC_PAUSE)) / 2));
			if (kPressed & KEY_A)
				audio_togglePlayback(music);
		}
		else
		{
			screen_draw_texture(TEXTURE_MUSIC_PLAY, ((320 - screen_get_texture_width(TEXTURE_MUSIC_PLAY)) / 2) - 2, ((240 - screen_get_texture_height(TEXTURE_MUSIC_PLAY)) / 2));
			if (kPressed & KEY_A)
				audio_togglePlayback(music);
		}

		screen_select(GFX_TOP);
		screen_draw_texture(TEXTURE_MUSIC_TOP_BG, 0, 0);

		drawWifiStatus(270, 2);
		drawBatteryStatus(295, 2);
		digitalTime();

		screen_draw_stringf(5, 25, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%s", fileName);

		screen_end_frame();

		if (kPressed & KEY_B)
		{
			// wait(100000000);
			break;
		}

		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}

	audio_stop(music);
}
