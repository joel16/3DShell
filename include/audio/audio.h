#ifndef AUDIO_H
#define AUDIO_H

#include <3ds.h>

extern volatile bool stop;

enum channel_e
{
	BGM,
	SFX
};

enum file_types
{
	FILE_TYPE_ERROR,
	FILE_TYPE_BCSTM,
	FILE_TYPE_WAV,
	FILE_TYPE_FLAC,
	FILE_TYPE_VORBIS,
	FILE_TYPE_MP3
};

struct decoder_fn
{
	int (* init)(const char *file);
	u32 (* rate)(void);
	u8 (* channels)(void);
	size_t buffSize;
	u64 (* decode)(void*);
	void (* exit)(void);
};

bool Audio_IsPaused(enum channel_e);
bool Audio_TogglePlayback(enum channel_e);
void Audio_StopPlayback(void);
bool Audio_IsPlaying(void);
enum file_types Audio_GetMusicFileType(const char *file);
void Audio_PlayFile(void *path);

#endif
