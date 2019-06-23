#include <string.h>

#include "audio.h"
#include "fs.h"

#include "3dsaudiolib.h"
#include "flac.h"
#include "mp3.h"
#include "ogg.h"
#include "audio/opus.h"
#include "wav.h"
#include "xm.h"

bool playing = true;
Audio_Metadata metadata = {0};
static Audio_Metadata empty = {0};

enum Audio_FileType {
	FILE_TYPE_NONE = 0,
	FILE_TYPE_FLAC = 1,
	FILE_TYPE_MP3 = 2,
	FILE_TYPE_OGG = 3,
	FILE_TYPE_OPUS = 4,
	FILE_TYPE_WAV = 5,
	FILE_TYPE_XM = 6
};

static enum Audio_FileType file_type = FILE_TYPE_NONE;

static u32 Audio_GetSampleRate(void) {
	u32 sample_rate = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			sample_rate = FLAC_GetSampleRate();
			break;

		case FILE_TYPE_MP3:
			sample_rate = MP3_GetSampleRate();
			break;

		case FILE_TYPE_OGG:
			sample_rate = OGG_GetSampleRate();
			break;

		case FILE_TYPE_OPUS:
			sample_rate = OPUS_GetSampleRate();
			break;

		case FILE_TYPE_WAV:
			sample_rate = WAV_GetSampleRate();
			break;

		case FILE_TYPE_XM:
			sample_rate = XM_GetSampleRate();
			break;

		default:
			break;
	}

	return sample_rate;
}

static u8 Audio_GetChannels(void) {
	u8 channels = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			channels = FLAC_GetChannels();
			break;

		case FILE_TYPE_MP3:
			channels = MP3_GetChannels();
			break;

		case FILE_TYPE_OGG:
			channels = OGG_GetChannels();
			break;

		case FILE_TYPE_OPUS:
			channels = OPUS_GetChannels();
			break;

		case FILE_TYPE_WAV:
			channels = WAV_GetChannels();
			break;

		case FILE_TYPE_XM:
			channels = XM_GetChannels();
			break;

		default:
			break;
	}

	return channels;
}

void Audio_Callback(void *userdata, void *stream, int length) {
	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Decode(stream, length / (Audio_GetChannels() * sizeof(s16)), userdata);
			break;

		case FILE_TYPE_MP3:
			MP3_Decode(stream, length / (Audio_GetChannels() * sizeof(s16)), userdata);
			break;

		case FILE_TYPE_OGG:
			OGG_Decode(stream, length / (Audio_GetChannels() * sizeof(s16)), userdata);
			break;

		case FILE_TYPE_OPUS:
			OPUS_Decode(stream, length / (Audio_GetChannels() * sizeof(s16)), userdata);
			break;

		case FILE_TYPE_WAV:
			WAV_Decode(stream, length / (Audio_GetChannels() * sizeof(s16)), userdata);
			break;

		case FILE_TYPE_XM:
			XM_Decode(stream, length / (Audio_GetChannels() * sizeof(s16)), userdata);
			break;

		default:
			break;
	}
}

static const char *Audio_GetFileExt(const char *filename) {
	const char *dot = strrchr(filename, '.');
	
	if (!dot || dot == filename)
		return "";
	
	return dot + 1;
}

void Audio_Init(const char *path) {
	playing = true;

	if (!strncasecmp(Audio_GetFileExt(path), "flac", 4))
		file_type = FILE_TYPE_FLAC;
	else if (!strncasecmp(Audio_GetFileExt(path), "mp3", 3))
		file_type = FILE_TYPE_MP3;
	else if (!strncasecmp(Audio_GetFileExt(path), "ogg", 3))
		file_type = FILE_TYPE_OGG;
	else if (!strncasecmp(Audio_GetFileExt(path), "opus", 4))
		file_type = FILE_TYPE_OPUS;
	else if (!strncasecmp(Audio_GetFileExt(path), "wav", 3))
		file_type = FILE_TYPE_WAV;
	else if ((!strncasecmp(Audio_GetFileExt(path), "it", 2)) || (!strncasecmp(Audio_GetFileExt(path), "mod", 3))
		|| (!strncasecmp(Audio_GetFileExt(path), "s3m", 3)) || (!strncasecmp(Audio_GetFileExt(path), "xm", 2)))
		file_type = FILE_TYPE_XM;

	u32 samples = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Init(path);
			samples = 1024;
			break;

		case FILE_TYPE_MP3:
			MP3_Init(path);
			samples = 4096;
			break;

		case FILE_TYPE_OGG:
			OGG_Init(path);
			samples = 4096;
			break;

		case FILE_TYPE_OPUS:
			OPUS_Init(path);
			samples = 960;
			break;

		case FILE_TYPE_WAV:
			WAV_Init(path);
			samples = 4096;
			break;

		case FILE_TYPE_XM:
			XM_Init(path);
			samples = 4096;
			break;

		default:
			break;
	}

	_3dsAudioInit(Audio_GetChannels(), Audio_GetSampleRate(), samples);
	_3dsAudioCreateThread();
}

bool Audio_IsPaused(void) {
	return ndspChnIsPaused(0);
}

void Audio_Pause(void) {
	ndspChnSetPaused(0, !(ndspChnIsPaused(0)));
}

void Audio_Stop(void) {
	playing = !playing;
}

u64 Audio_GetPosition(void) {
	u64 position = -1;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			position = FLAC_GetPosition();
			break;

		case FILE_TYPE_MP3:
			position = MP3_GetPosition();
			break;

		case FILE_TYPE_OGG:
			position = OGG_GetPosition();
			break;

		case FILE_TYPE_OPUS:
			position = OPUS_GetPosition();
			break;

		case FILE_TYPE_WAV:
			position = WAV_GetPosition();
			break;

		case FILE_TYPE_XM:
			position = XM_GetPosition();
			break;

		default:
			break;
	}

	return position;
}

u64 Audio_GetLength(void) {
	u64 length = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			length = FLAC_GetLength();
			break;

		case FILE_TYPE_MP3:
			length = MP3_GetLength();
			break;

		case FILE_TYPE_OGG:
			length = OGG_GetLength();
			break;

		case FILE_TYPE_OPUS:
			length = OPUS_GetLength();
			break;

		case FILE_TYPE_WAV:
			length = WAV_GetLength();
			break;

		case FILE_TYPE_XM:
			length = XM_GetLength();
			break;

		default:
			break;
	}

	return length;
}

u64 Audio_GetPositionSeconds(void) {
	return (Audio_GetPosition()/Audio_GetSampleRate());
}

u64 Audio_GetLengthSeconds(void) {
	return (Audio_GetLength()/Audio_GetSampleRate());
}

void Audio_Term(void) {
	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Term();
			break;

		case FILE_TYPE_MP3:
			MP3_Term();
			break;

		case FILE_TYPE_OGG:
			OGG_Term();
			break;

		case FILE_TYPE_OPUS:
			OPUS_Term();
			break;

		case FILE_TYPE_WAV:
			WAV_Term();
			break;

		case FILE_TYPE_XM:
			XM_Term();
			break;

		default:
			break;
	}

	playing = true;

	// Clear metadata struct
	metadata = empty;
	_3dsAudioExitThread();
	_3dsAudioEnd();
}
