#include <3ds.h>
#include <string.h>
#include <stdlib.h>

#include "audio/audio.h"

bool audio_isPlaying(enum channel_e channel)
{
	return ndspChnIsPlaying(channel);
}

bool audio_isPaused(enum channel_e channel)
{
	return ndspChnIsPaused(channel);
}

bool audio_togglePlayback(enum channel_e channel)
{
	bool paused = ndspChnIsPaused(channel);
	ndspChnSetPaused(channel, !paused);
	return !paused;
}
