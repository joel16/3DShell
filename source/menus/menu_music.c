#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "flac.h"
#include "mp3.h"
#include "vorbis.h"
#include "wav.h"

#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "menu_music.h"
#include "pp2d.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static volatile bool stop = true;

/**
 * Stops current playback. Playback thread should exit as a result.
 */
static void Music_StopPlayback(void)
{
	stop = true;
}

/**
 * Returns whether music is playing or paused.
 */
bool Music_IsPlaying(void)
{
	return !stop;
}

/**
 * Obtains audio file type. Lifted from ctrmus with permission.
 *
 * \param	file	File location.
 * \return			file_types enum or 0 on unsupported file or error.
 */
enum file_types Music_GetMusicFileType(const char * file)
{
	Handle handle;
	
	u32 fileSig = 0, bytesRead = 0;
	u64 offset = 0;
	
	enum file_types file_type = FILE_TYPE_ERROR;
	
	/* Failure opening file */
	if (R_FAILED(FS_Open(&handle, archive, file, FS_OPEN_READ))) 
	{
		FSFILE_Close(handle);
		return -1;
	}
	
	if (R_FAILED(FSFILE_Read(handle, &bytesRead, offset, &fileSig, 4)))
	{
		FSFILE_Close(handle);
		return -2;
	}
	
	offset += bytesRead;
	
	switch(fileSig)
	{
		// "RIFF"
		case 0x46464952:
			if (isWav(file) == 0)
				file_type = FILE_TYPE_WAV;
			break;

		// "fLaC"
		case 0x43614c66:
			file_type = FILE_TYPE_FLAC;
			break;

		// "OggS"
		case 0x5367674F:
			if (isFlac(file) == 0)
				file_type = FILE_TYPE_FLAC;
			else if (isVorbis(file) == 0)
				file_type = FILE_TYPE_VORBIS;

			break;
			
		// MP3 file with an ID3v2 container
		default:
			if ((fileSig << 16) == 0xFBFF0000 || (fileSig << 16) == 0xFAFF0000 || (fileSig << 8) == 0x33444900)
			{
				file_type = FILE_TYPE_MP3;
				break;
			}
	}

	FSFILE_Close(handle);
	return file_type;
}

/**
 * Should only be called from a new thread only, and have only one playback
 * thread at time. This function has not been written for more than one
 * playback thread in mind.
 *
 * \param	pathIn	File location.
 */
static void Music_PlayFile(void * pathIn)
{
	struct decoder_fn decoder;
	const char * file = pathIn;
	s16 * buffer1 = NULL;
	s16 * buffer2 = NULL;
	ndspWaveBuf waveBuf[2];
	bool lastbuf = false;
	Result ret = -1;

	/* Reset previous stop command */
	stop = false;

	switch(Music_GetMusicFileType(file))
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

	if ((ret = (*decoder.init)(file)) != 0)
		goto out;

	if ((*decoder.channels)() > 2 || (*decoder.channels)() < 1)
		goto out;

	buffer1 = linearAlloc(decoder.buffSize * sizeof(s16));
	buffer2 = linearAlloc(decoder.buffSize * sizeof(s16));

	ndspChnReset(SFX);
	ndspChnWaveBufClear(SFX);
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnSetInterp(SFX, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(SFX, (*decoder.rate)());
	ndspChnSetFormat(SFX, (*decoder.channels)() == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

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
		if ((lastbuf == true) && (waveBuf[0].status == NDSP_WBUF_DONE) && (waveBuf[1].status == NDSP_WBUF_DONE))
			break;

		if ((ndspChnIsPaused(SFX) == true) || (lastbuf == true))
			continue;

		if (waveBuf[0].status == NDSP_WBUF_DONE)
		{
			size_t read = (*decoder.decode)(&buffer1[0]);

			if (read <= 0)
			{
				lastbuf = true;
				continue;
			}
			else if (read < decoder.buffSize)
				waveBuf[0].nsamples = read / (*decoder.channels)();

			ndspChnWaveBufAdd(SFX, &waveBuf[0]);
		}

		if (waveBuf[1].status == NDSP_WBUF_DONE)
		{
			size_t read = (*decoder.decode)(&buffer2[0]);

			if (read <= 0)
			{
				lastbuf = true;
				continue;
			}
			else if (read < decoder.buffSize)
				waveBuf[1].nsamples = read / (*decoder.channels)();

			ndspChnWaveBufAdd(SFX, &waveBuf[1]);
		}

		if (R_FAILED(DSP_FlushDataCache(buffer1, decoder.buffSize * sizeof(s16))))
			return;
		if (R_FAILED(DSP_FlushDataCache(buffer2, decoder.buffSize * sizeof(s16))))
			return;
	}

	(*decoder.exit)();

out:
	Music_StopPlayback();
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
void Music_Player(char * path)
{
	s32 prio;
	static Thread thread = NULL;

	/* Reset previous stop command */
	stop = false;

	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	thread = threadCreate(Music_PlayFile, path, 32 * 1024, prio - 1, -2, false);
	
	File * file = Dirlist_GetFileIndex(position);
	bool isMP3 = (strncasecmp(file->ext, "mp3", 3) == 0);
	
	while (Music_IsPlaying())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		if ((kDown & KEY_A) || ((touchInRect(114, 76, 204, 164)) && (kDown & KEY_TOUCH)))
			Audio_TogglePlayback(SFX);

		pp2d_begin_draw(GFX_BOTTOM, GFX_LEFT);

			pp2d_draw_texture(TEXTURE_MUSIC_BOTTOM_BG, 0, 0);

			if (!(Audio_IsPaused(SFX)))
				pp2d_draw_texture(TEXTURE_MUSIC_PAUSE, ((320.0 - pp2d_get_texture_width(TEXTURE_MUSIC_PAUSE)) / 2.0) - 2, 
					((240.0 - pp2d_get_texture_height(TEXTURE_MUSIC_PAUSE)) / 2.0));
			else
				pp2d_draw_texture(TEXTURE_MUSIC_PLAY, ((320.0 - pp2d_get_texture_width(TEXTURE_MUSIC_PLAY)) / 2.0), 
					((240.0 - pp2d_get_texture_height(TEXTURE_MUSIC_PLAY)) / 2.0));

		pp2d_end_draw();

		pp2d_begin_draw(GFX_TOP, GFX_LEFT);

			pp2d_draw_texture(TEXTURE_MUSIC_TOP_BG, 0, 0);

			StatusBar_DisplayBar();

			if (isMP3) // Only print out ID3 tag info for MP3
			{	
				pp2d_draw_textf(5, 20, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%s", fileName);
				pp2d_draw_textf(5, 36, 0.45f, 0.45f, RGBA8(255, 255, 255, 255), "%s", ID3.artist);
		
				pp2d_draw_textf(184, 64, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%.30s", ID3.title);
				pp2d_draw_textf(184, 84, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%.30s", ID3.album);
				pp2d_draw_textf(184, 104, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%.30s", ID3.year);
				pp2d_draw_textf(184, 124, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%.30s", ID3.genre);	
			}
		
			else
				pp2d_draw_textf(5, 25, 0.5f, 0.5f, RGBA8(255, 255, 255, 255), "%s", fileName);	
		
		pp2d_end_draw();

		if (kDown & KEY_B)
		{
			wait(10);
			Music_StopPlayback();
			break;
		}

		if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
			Screenshot_Capture();
	}

	threadJoin(thread, U64_MAX);
	threadFree(thread);
	
	// Clear ID3
	memset(ID3.artist, 0, 30);
	memset(ID3.title, 0, 30);
	memset(ID3.album, 0, 30);
	memset(ID3.year, 0, 4);
	memset(ID3.genre, 0, 30);

	return;
}