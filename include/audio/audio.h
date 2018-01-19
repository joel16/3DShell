#ifndef AUDIO_H
#define AUDIO_H

#include "menu_music.h"

enum channel_e
{
	BGM = 0,
	SFX
};

bool audio_isPlaying(enum channel_e);
bool audio_isPaused(enum channel_e);
bool audio_togglePlayback(enum channel_e);

#endif
