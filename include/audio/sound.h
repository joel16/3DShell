#ifndef SOUND_H
#define SOUND_H

/*
	Code from undertale_techdemo by  Xavyrr and kitling. 
*/

#include <3ds.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

extern bool isPlaying;

struct sound {
    const char* filename;
    OggVorbis_File vf;
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

enum channel {
    BGM = 0,
    SFX
};

struct sound * sound_create(enum channel chan);
void audio_load_ogg(const char *audio, struct sound *sound);
void sound_stop(struct sound *sound);

#endif
