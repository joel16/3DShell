/* Obtained from ctrmus source with permission. */

#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

#include "music.h"

void setVorbis(struct decoder_fn * decoder);

int initVorbis(const char * file);

u32 rateVorbis(void);

u8 channelVorbis(void);

u64 decodeVorbis(void * buffer);

void exitVorbis(void);

int playVorbis(const char * in);

u64 fillVorbisBuffer(char * bufferOut);

int isVorbis(const char * in);