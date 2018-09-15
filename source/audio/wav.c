/* Obtained from ctrmus source with permission. */

#define DR_WAV_IMPLEMENTATION
#include <dr_libs/dr_wav.h>

#include "fs.h"
#include "wav.h"

static const drwav_uint64 buffSize = 16 * 1024;
static drwav* pWav;
static drwav_uint64 samplesRead;

static int WAV_Init(const char* filename)
{
	pWav = drwav_open_file(filename);
	return pWav == NULL ? -1 : 0;
}

static drwav_uint32 WAV_GetSampleRate(void)
{
	return pWav->sampleRate;
}

static drwav_uint8 WAV_GetChannels(void)
{
	return pWav->channels;
}

static drwav_uint64 WAV_Decode(void *buffer)
{
	samplesRead = drwav_read_s16(pWav, buffSize, buffer);
	return samplesRead;
}

static void WAV_Term(void)
{
	drwav_close(pWav);
}

void WAV_SetDecoder(struct decoder_fn *decoder)
{
	decoder->init = &WAV_Init;
	decoder->rate = &WAV_GetSampleRate;
	decoder->channels = &WAV_GetChannels;
	decoder->buffSize = buffSize;
	decoder->decode = &WAV_Decode;
	decoder->exit = &WAV_Term;
}

int WAV_Validate(const char *in)
{
	int err = -1;
	drwav *wav_fiile = drwav_open_file(in);

	if (wav_fiile != NULL)
		err = 0;

	drwav_close(wav_fiile);
	return err;
}
