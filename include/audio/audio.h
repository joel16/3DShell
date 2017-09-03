#ifndef AUDIO_H
#define AUDIO_H

#include "mpg123.h"

#include <3ds.h>
#include <tremor/ivorbisfile.h>

struct audio
{
	const char * filename;
	OggVorbis_File vf;
	mpg123_handle * mpg123;
	ndspWaveBuf waveBuf[2];
	float mix[12];
	long status;
	int section;
	int channel;
	bool block;
	unsigned long block_pos;
	Thread thread;
	LightEvent stopEvent;
};

enum channel
{
	BGM = 0,
	SFX
};

bool audio_isPlaying(struct audio * audio);
bool audio_isPaused(struct audio * audio);
bool audio_togglePlayback(struct audio * audio);
void audio_stop(struct audio * audio);

#endif
