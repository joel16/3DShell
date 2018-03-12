#include <3ds.h>
#include <string.h>
#include <stdlib.h>

#include "audio.h"

bool Audio_IsPlaying(enum channel_e channel)
{
	return ndspChnIsPlaying(channel);
}

bool Audio_IsPaused(enum channel_e channel)
{
	return ndspChnIsPaused(channel);
}

bool Audio_TogglePlayback(enum channel_e channel)
{
	ndspChnSetPaused(channel, !(ndspChnIsPaused(channel)));
	return !(ndspChnIsPaused(channel));
}