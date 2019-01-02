/* Obtained from ctrmus source with permission. */

#define DR_FLAC_IMPLEMENTATION
#include <dr_libs/dr_flac.h>

#include "flac.h"

static drflac *pFlac;
static const drflac_uint64 buffSize = 16 * 1024;

static int FLAC_Init(const char *file) {
	pFlac = drflac_open_file(file);
	return pFlac == NULL ? -1 : 0;
}

static drflac_uint32 FLAC_GetSampleRate(void) {
	return pFlac->sampleRate;
}

static drflac_uint8 FLAC_GetChannels(void) {
	return pFlac->channels;
}

static drflac_uint64 FLAC_Decode(void *buffer) {
	return drflac_read_pcm_frames_s16(pFlac, buffSize, (drflac_int16 *)buffer);
}

static void FLAC_Term(void) {
	drflac_close(pFlac);
}

void FLAC_SetDecoder(struct decoder_fn *decoder) {
	decoder->init = &FLAC_Init;
	decoder->rate = &FLAC_GetSampleRate;
	decoder->channels = &FLAC_GetChannels;
	decoder->buffSize = buffSize;
	decoder->decode = &FLAC_Decode;
	decoder->exit = &FLAC_Term;
}

int FLAC_Validate(const char *file) {
	drflac *pFlac = drflac_open_file(file);

	if (pFlac == NULL) {
		drflac_close(pFlac);
		return -1;	
	}
	
	drflac_close(pFlac);
	return 0;
}
