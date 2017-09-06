#include <3ds.h>
#include <stdio.h>

#include "audio.h"
#include "audio/flac.h"
#include "audio/mp3.h"
#include "audio/vorbis.h"
#include "audio/wav.h"
#include "clock.h"
#include "common.h"
#include "graphics/screen.h"
#include "music.h"
#include "power.h"
#include "screenshot.h"
#include "utils.h"
#include "wifi.h"

static volatile bool stop = true;

/**
 * Stops current playback. Playback thread should exit as a result.
 */
static void stopPlayback(void)
{
	stop = true;
}

/**
 * Returns whether music is playing or paused.
 */
static bool isPlaying(void)
{
	return !stop;
}

/**
 * Obtains audio file type. Lifted from ctrmus with permission.
 *
 * \param	file	File location.
 * \return			file_types enum or 0 on unsupported file or error.
 */
enum file_types getMusicFileType(const char *file)
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
			if(isFlac(file) == 0)
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
 * Should only be called from a new thread only, and have only one playback
 * thread at time. This function has not been written for more than one
 * playback thread in mind.
 *
 * \param	pathIn	File location.
 */
static void playFile(void* pathIn)
{
	struct decoder_fn decoder;
	const char*		file = pathIn;
	int16_t*		buffer1 = NULL;
	int16_t*		buffer2 = NULL;
	ndspWaveBuf		waveBuf[2];
	bool			lastbuf = false;
	int				ret = -1;

	/* Reset previous stop command */
	stop = false;

	switch(getMusicFileType(file))
	{
		case FILE_TYPE_WAV:
			setWav(&decoder);
			break;

		case FILE_TYPE_FLAC:
			setFlac(&decoder);
			break;

		case FILE_TYPE_MP3:
			setMp3(&decoder);
			break;

		case FILE_TYPE_VORBIS:
			setVorbis(&decoder);
			break;

		default:
			goto out;
	}

	if((ret = (*decoder.init)(file)) != 0)
		goto out;

	if((*decoder.channels)() > 2 || (*decoder.channels)() < 1)
		goto out;

	buffer1 = linearAlloc(decoder.buffSize * sizeof(int16_t));
	buffer2 = linearAlloc(decoder.buffSize * sizeof(int16_t));

	ndspChnReset(SFX);
	ndspChnWaveBufClear(SFX);
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnSetInterp(SFX, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(SFX, (*decoder.rate)());
	ndspChnSetFormat(SFX,
			(*decoder.channels)() == 2 ? NDSP_FORMAT_STEREO_PCM16 :
			NDSP_FORMAT_MONO_PCM16);

	memset(waveBuf, 0, sizeof(waveBuf));
	waveBuf[0].nsamples = (*decoder.decode)(&buffer1[0]) / (*decoder.channels)();
	waveBuf[0].data_vaddr = &buffer1[0];
	ndspChnWaveBufAdd(SFX, &waveBuf[0]);

	waveBuf[1].nsamples = (*decoder.decode)(&buffer2[0]) / (*decoder.channels)();
	waveBuf[1].data_vaddr = &buffer2[0];
	ndspChnWaveBufAdd(SFX, &waveBuf[1]);

	/**
	 * There may be a chance that the music has not started by the time we get
	 * to the while loop. So we ensure that music has started here.
	 */
	while(ndspChnIsPlaying(SFX) == false);

	while(stop == false)
	{
		svcSleepThread(100 * 1000);

		/* When the last buffer has finished playing, break. */
		if(lastbuf == true && waveBuf[0].status == NDSP_WBUF_DONE &&
				waveBuf[1].status == NDSP_WBUF_DONE)
			break;

		if(ndspChnIsPaused(SFX) == true || lastbuf == true)
			continue;

		if(waveBuf[0].status == NDSP_WBUF_DONE)
		{
			size_t read = (*decoder.decode)(&buffer1[0]);

			if(read <= 0)
			{
				lastbuf = true;
				continue;
			}
			else if(read < decoder.buffSize)
				waveBuf[0].nsamples = read / (*decoder.channels)();

			ndspChnWaveBufAdd(SFX, &waveBuf[0]);
		}

		if(waveBuf[1].status == NDSP_WBUF_DONE)
		{
			size_t read = (*decoder.decode)(&buffer2[0]);

			if(read <= 0)
			{
				lastbuf = true;
				continue;
			}
			else if(read < decoder.buffSize)
				waveBuf[1].nsamples = read / (*decoder.channels)();

			ndspChnWaveBufAdd(SFX, &waveBuf[1]);
		}

		DSP_FlushDataCache(buffer1, decoder.buffSize * sizeof(int16_t));
		DSP_FlushDataCache(buffer2, decoder.buffSize * sizeof(int16_t));
	}

	(*decoder.exit)();

out:
	stopPlayback();
	linearFree(buffer1);
	linearFree(buffer2);

	threadExit(0);
	return;
}

/**
 * Play an audio file.
 *
 * \param path	File path.
 */
void musicPlayer(char * path)
{
	s32 prio;
	static Thread thread = NULL;

	/* Reset previous stop command */
	stop = false;

	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	thread = threadCreate(playFile, path, 32 * 1024, prio - 1, -2, false);

	while (isPlaying())
	{
		hidScanInput();

		if (kPressed & KEY_A)
			audio_togglePlayback(SFX);

		screen_begin_frame();
		screen_select(GFX_BOTTOM);
		screen_draw_texture(TEXTURE_MUSIC_BOTTOM_BG, 0, 0);

		if (!(audio_isPaused(SFX)))
			screen_draw_texture(TEXTURE_MUSIC_PAUSE, ((320 - screen_get_texture_width(TEXTURE_MUSIC_PAUSE)) / 2) - 2, ((240 - screen_get_texture_height(TEXTURE_MUSIC_PAUSE)) / 2));
		else
			screen_draw_texture(TEXTURE_MUSIC_PLAY, ((320 - screen_get_texture_width(TEXTURE_MUSIC_PLAY)) / 2) - 2, ((240 - screen_get_texture_height(TEXTURE_MUSIC_PLAY)) / 2));

		screen_select(GFX_TOP);
		screen_draw_texture(TEXTURE_MUSIC_TOP_BG, 0, 0);

		drawWifiStatus();
		drawBatteryStatus();
		digitalTime();

		screen_draw_stringf(5, 25, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%s", path);

		screen_end_frame();

		if (kPressed & KEY_B)
		{
			stopPlayback();
			break;
		}

		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}

	threadJoin(thread, U64_MAX);
	threadFree(thread);

	return;
}
