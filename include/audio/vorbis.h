/* Obtained from ctrmus source with permission. */

#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

#include "music.h"

void setVorbis(struct decoder_fn* decoder);

int initVorbis(const char* file);

uint32_t rateVorbis(void);

uint8_t channelVorbis(void);

uint64_t decodeVorbis(void* buffer);

void exitVorbis(void);

int playVorbis(const char* in);

uint64_t fillVorbisBuffer(char* bufferOut);

int isVorbis(const char* in);
