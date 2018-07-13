/* Obtained from ctrmus source with permission. */

#define DR_FLAC_IMPLEMENTATION
#include <dr_libs/dr_flac.h>

#include "flac.h"

static drflac *pFlac;
static const drflac_uint64 buffSize = 16 * 1024;
static drflac_uint64 samplesRead;

/**
 *Set decoder parameters for flac.
 *
 *\param	decoder Structure to store parameters.
 */
void setFlac(struct decoder_fn *decoder)
{
	decoder->init = &initFlac;
	decoder->rate = &rateFlac;
	decoder->channels = &channelFlac;
	decoder->buffSize = buffSize;
	decoder->decode = &decodeFlac;
	decoder->exit = &exitFlac;
}

/**
 *Initialise Flac decoder.
 *
 *\param	file	Location of flac file to play.
 *\return			0 on success, else failure.
 */
int initFlac(const char *file)
{
	pFlac = drflac_open_file(file);

	return pFlac == NULL ? -1 : 0;
}

/**
 *Get sampling rate of Flac file.
 *
 *\return	Sampling rate.
 */
u32 rateFlac(void)
{
	return pFlac->sampleRate;
}

/**
 *Get number of channels of Flac file.
 *
 *\return	Number of channels for opened file.
 */
u8 channelFlac(void)
{
	return pFlac->channels;
}

/**
 *Decode part of open Flac file.
 *
 *\param buffer	Decoded output.
 *\return			Samples read for each channel.
 */
u64 decodeFlac(void *buffer)
{
	samplesRead = drflac_read_s16(pFlac, buffSize, buffer);
	return samplesRead;
}

/**
 *Free Flac decoder.
 */
void exitFlac(void)
{
	drflac_close(pFlac);
}

/**
 *Checks if the input file is Flac
 *
 *\param in	Input file.
 *\return		0 if Flac file, else not or failure.
 */
int isFlac(const char *in)
{
	int err = -1;
	drflac *pFlac = drflac_open_file(in);

	if (pFlac != NULL)
		err = 0;

	drflac_close(pFlac);
	return err;
}
