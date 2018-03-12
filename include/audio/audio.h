#ifndef AUDIO_H
#define AUDIO_H

#include "menu_music.h"

enum channel_e
{
	BGM = 0,
	SFX
};

bool Audio_IsPlaying(enum channel_e);
bool Audio_IsPaused(enum channel_e);
bool Audio_TogglePlayback(enum channel_e);

#endif
