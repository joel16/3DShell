#include <3ds.h>
#include <string.h>
#include <stdlib.h>

#include "audio.h"
#include "audio_opus.h"
#include "flac.h"
#include "mp3.h"
#include "vorbis.h"
#include "wav.h"

#include "fs.h"

volatile bool stop = true;
struct decoder_fn decoder;

/*bool Audio_IsPlaying(enum channel_e channel)
{
	return ndspChnIsPlaying(channel);
}*/

bool Audio_IsPaused(enum channel_e channel) {
	return ndspChnIsPaused(channel);
}

bool Audio_TogglePlayback(enum channel_e channel) {
	ndspChnSetPaused(channel, !(ndspChnIsPaused(channel)));
	return !(ndspChnIsPaused(channel));
}

void Audio_StopPlayback(void) {
	stop = true;
}

bool Audio_IsPlaying(void) {
	return !stop;
}

enum file_types Audio_GetMusicFileType(const char *file) {
	Handle handle;
	
	u32 fileSig = 0, bytesRead = 0;
	u64 offset = 0;
	
	enum file_types file_type = FILE_TYPE_ERROR;
	
	/* Failure opening file */
	if (R_FAILED(FS_OpenFile(&handle, archive, file, FS_OPEN_READ, 0))) {
		FSFILE_Close(handle);
		return -1;
	}
	
	if (R_FAILED(FSFILE_Read(handle, &bytesRead, offset, &fileSig, 4))) {
		FSFILE_Close(handle);
		return -2;
	}
	
	offset += bytesRead;
	
	switch(fileSig) {
		// "RIFF"
		case 0x46464952:
			if (WAV_Validate(file) == 0)
				file_type = FILE_TYPE_WAV;
			break;

		// "fLaC"
		case 0x43614c66:
			file_type = FILE_TYPE_FLAC;
			break;

		// "OggS"
		case 0x5367674F:
			if (Opus_Validate(file) == 0)
				file_type = FILE_TYPE_OPUS;
			else if (FLAC_Validate(file) == 0)
				file_type = FILE_TYPE_FLAC;
			else if (VORBIS_Validate(file) == 0)
				file_type = FILE_TYPE_VORBIS;

			break;
			
		// MP3 file with an ID3v2 container
		default:
			if ((fileSig << 16) == 0xFBFF0000 || (fileSig << 16) == 0xFAFF0000 || (fileSig << 8) == 0x33444900) {
				file_type = FILE_TYPE_MP3;
				break;
			}
	}

	FSFILE_Close(handle);
	return file_type;
}

int Audio_GetPosition(void) {
	return (*decoder.position)();
}

int Audio_GetLength(void) {
	return (*decoder.length)();
}

void Audio_PlayFile(void *path) {
	const char *file = path;
	s16 *buffer1 = NULL;
	s16 *buffer2 = NULL;
	ndspWaveBuf waveBuf[2];
	bool lastbuf = false;
	Result ret = -1;

	/* Reset previous stop command */
	stop = false;

	switch(Audio_GetMusicFileType(file)) {
		case FILE_TYPE_WAV:
			WAV_SetDecoder(&decoder);
			break;

		case FILE_TYPE_FLAC:
			FLAC_SetDecoder(&decoder);
			break;

		case FILE_TYPE_OPUS:
			Opus_SetDecoder(&decoder);
			break;

		case FILE_TYPE_MP3:
			MP3_SetDecoder(&decoder);
			break;

		case FILE_TYPE_VORBIS:
			VORBIS_SetDecoder(&decoder);
			break;

		default:
			goto out;
	}

	if ((ret = (*decoder.init)(file)) != 0)
		goto out;

	if ((*decoder.channels)() > 2 || (*decoder.channels)() < 1)
		goto out;

	buffer1 = linearAlloc(decoder.buffSize *sizeof(s16));
	buffer2 = linearAlloc(decoder.buffSize *sizeof(s16));

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
	 *There may be a chance that the music has not started by the time we get
	 *to the while loop. So we ensure that music has started here.
	 */
	while(ndspChnIsPlaying(SFX) == false);

	while(stop == false) {
		svcSleepThread(100 *1000);

		/* When the last buffer has finished playing, break. */
		if ((lastbuf == true) && (waveBuf[0].status == NDSP_WBUF_DONE) && (waveBuf[1].status == NDSP_WBUF_DONE))
			break;

		if ((ndspChnIsPaused(SFX) == true) || (lastbuf == true))
			continue;

		if (waveBuf[0].status == NDSP_WBUF_DONE) {
			size_t read = (*decoder.decode)(&buffer1[0]);

			if (read <= 0) {
				lastbuf = true;
				continue;
			}
			else if (read < decoder.buffSize)
				waveBuf[0].nsamples = read / (*decoder.channels)();

			ndspChnWaveBufAdd(SFX, &waveBuf[0]);
		}

		if (waveBuf[1].status == NDSP_WBUF_DONE) {
			size_t read = (*decoder.decode)(&buffer2[0]);

			if (read <= 0) {
				lastbuf = true;
				continue;
			}
			else if (read < decoder.buffSize)
				waveBuf[1].nsamples = read / (*decoder.channels)();

			ndspChnWaveBufAdd(SFX, &waveBuf[1]);
		}

		if (R_FAILED(DSP_FlushDataCache(buffer1, decoder.buffSize *sizeof(s16))))
			return;
		if (R_FAILED(DSP_FlushDataCache(buffer2, decoder.buffSize *sizeof(s16))))
			return;
	}

	(*decoder.exit)();

out:
	Audio_StopPlayback();
	linearFree(buffer1);
	linearFree(buffer2);

	threadExit(0);
	return;
}
