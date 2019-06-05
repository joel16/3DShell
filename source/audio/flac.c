#include "audio.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

static drflac *flac;
static drflac_uint64 frames_read = 0;

int FLAC_Init(const char *path) {
	flac = drflac_open_file(path);
	if (flac == NULL)
		return -1;

	return 0;
}

u32 FLAC_GetSampleRate(void) {
	return flac->sampleRate;
}

u8 FLAC_GetChannels(void) {
	return flac->channels;
}

void FLAC_Decode(void *buf, unsigned int length, void *userdata) {
	frames_read += drflac_read_s16(flac, (drflac_uint64)length * flac->channels, (drflac_int16 *)buf);
	
	if (frames_read >= flac->totalSampleCount)
		playing = false;
}

u64 FLAC_GetPosition(void) {
	return (frames_read / flac->channels);
}

u64 FLAC_GetLength(void) {
	return (flac->totalSampleCount / flac->channels);
}

void FLAC_Term(void) {
	frames_read = 0;
	drflac_close(flac);
}
