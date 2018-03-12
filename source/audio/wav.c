/* Obtained from ctrmus source with permission. */

#define DR_WAV_IMPLEMENTATION
#include <dr_libs/dr_wav.h>

#include "fs.h"
#include "wav.h"

static const drwav_uint64 buffSize = 16 * 1024;
static drwav* pWav;
static drwav_uint64 samplesRead;

/**
 * Set decoder parameters for WAV.
 *
 * \param	decoder Structure to store parameters.
 */
void setWav(struct decoder_fn * decoder)
{
	decoder->init = &initWav;
	decoder->rate = &rateWav;
	decoder->channels = &channelWav;
	decoder->buffSize = buffSize;
	decoder->decode = &decodeWav;
	decoder->exit = &exitWav;
}

/**
 * Initialise WAV playback.
 *
 * \param	file	Location of WAV file to play.
 * \return			0 on success, else failure.
 */
int initWav(const char* filename)
{
	pWav = drwav_open_file(filename);

	return pWav == NULL ? -1 : 0;
}

/**
 * Get sampling rate of Wav file.
 *
 * \return	Sampling rate.
 */
u32 rateWav(void)
{
	return pWav->sampleRate;
}

/**
 * Get number of channels of Wav file.
 *
 * \return	Number of channels for opened file.
 */
u8 channelWav(void)
{
	return pWav->channels;
}

/**
 * Read part of open Wav file.
 *
 * \param buffer	Output.
 * \return			Samples read for each channel.
 */
u64 decodeWav(void * buffer)
{
	samplesRead = drwav_read_s16(pWav, buffSize, buffer);
	return samplesRead;
}

/**
 * Free Wav file.
 */
void exitWav(void)
{
	drwav_close(pWav);
}

/**
 * Checks if the input file is Wav
 *
 * \param in	Input file.
 * \return		0 if Wav file, else not or failure.
 */
int isWav(const char * in)
{
	int err = -1;
	drwav * testWav = drwav_open_file(in);

	if (testWav != NULL)
		err = 0;

	drwav_close(testWav);
	return err;
}
