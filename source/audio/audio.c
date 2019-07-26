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

enum Audio_FileType {
	FILE_TYPE_NONE = 0,
	FILE_TYPE_FLAC = 1,
	FILE_TYPE_MP3 = 2,
	FILE_TYPE_OGG = 3,
	FILE_TYPE_OPUS = 4,
	FILE_TYPE_WAV = 5,
	FILE_TYPE_XM = 6
};

typedef struct {
	int (* init)(const char *path);
	u32 (* rate)(void);
	u8 (* channels)(void);
	void (* decode)(void *buf, unsigned int length, void *userdata);
	u64 (* position)(void);
	u64 (* length)(void);
	u64 (* seek)(u64 index);
	void (* term)(void);
	u32 nsamples;
} Audio_Decoder;

bool playing = true;
Audio_Metadata metadata = {0};
static Audio_Metadata empty_metadata = {0};
static Audio_Decoder decoder = {0}, empty_decoder = {0};
static enum Audio_FileType file_type = FILE_TYPE_NONE;

void Audio_Callback(void *userdata, void *stream, int length) {
	if (playing == false || ndspChnIsPaused(0))
		memset(stream, 0, length / ((* decoder.channels)() * sizeof(s16)));
	else
		(* decoder.decode)(stream, length / ((* decoder.channels)() * sizeof(s16)), userdata);
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

	switch(file_type) {
		case FILE_TYPE_FLAC:
			decoder.init = FLAC_Init;
			decoder.rate = FLAC_GetSampleRate;
			decoder.channels = FLAC_GetChannels;
			decoder.decode = FLAC_Decode;
			decoder.position = FLAC_GetPosition;
			decoder.length = FLAC_GetLength;
			decoder.seek = FLAC_Seek;
			decoder.term = FLAC_Term;
			decoder.nsamples = 1024;
			break;

		case FILE_TYPE_MP3:
			decoder.init = MP3_Init;
			decoder.rate = MP3_GetSampleRate;
			decoder.channels = MP3_GetChannels;
			decoder.decode = MP3_Decode;
			decoder.position = MP3_GetPosition;
			decoder.length = MP3_GetLength;
			decoder.seek = MP3_Seek;
			decoder.term = MP3_Term;
			decoder.nsamples = 4096;
			break;

		case FILE_TYPE_OGG:
			decoder.init = OGG_Init;
			decoder.rate = OGG_GetSampleRate;
			decoder.channels = OGG_GetChannels;
			decoder.decode = OGG_Decode;
			decoder.position = OGG_GetPosition;
			decoder.length = OGG_GetLength;
			decoder.seek = OGG_Seek;
			decoder.term = OGG_Term;
			decoder.nsamples = 4096;
			break;

		case FILE_TYPE_OPUS:
			decoder.init = OPUS_Init;
			decoder.rate = OPUS_GetSampleRate;
			decoder.channels = OPUS_GetChannels;
			decoder.decode = OPUS_Decode;
			decoder.position = OPUS_GetPosition;
			decoder.length = OPUS_GetLength;
			decoder.seek = OPUS_Seek;
			decoder.term = OPUS_Term;
			decoder.nsamples = 960;
			break;

		case FILE_TYPE_WAV:
			decoder.init = WAV_Init;
			decoder.rate = WAV_GetSampleRate;
			decoder.channels = WAV_GetChannels;
			decoder.decode = WAV_Decode;
			decoder.position = WAV_GetPosition;
			decoder.length = WAV_GetLength;
			decoder.seek = WAV_Seek;
			decoder.term = WAV_Term;
			decoder.nsamples = 4096;
			break;

		case FILE_TYPE_XM:
			decoder.init = XM_Init;
			decoder.rate = XM_GetSampleRate;
			decoder.channels = XM_GetChannels;
			decoder.decode = XM_Decode;
			decoder.position = XM_GetPosition;
			decoder.length = XM_GetLength;
			decoder.seek = XM_Seek;
			decoder.term = XM_Term;
			decoder.nsamples = 4096;
			break;

		default:
			break;
	}

	(* decoder.init)(path);
	_3dsAudioInit((* decoder.channels)(), (* decoder.rate)(), decoder.nsamples);
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
	return (* decoder.position)();
}

u64 Audio_GetLength(void) {
	return (* decoder.length)();
}

u64 Audio_GetPositionSeconds(void) {
	return (Audio_GetPosition() / (* decoder.rate)());
}

u64 Audio_GetLengthSeconds(void) {
	return (Audio_GetLength() / (* decoder.rate)());
}

u64 Audio_Seek(u64 index) {
	return (* decoder.seek)(index);
}

void Audio_Term(void) {
	playing = true;

	_3dsAudioExitThread();
	_3dsAudioEnd();
	(* decoder.term)();

	// Clear metadata struct
	metadata = empty_metadata;
	decoder = empty_decoder;
}
