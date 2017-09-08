/* Obtained from ctrmus source with permission. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/vorbis.h"
#include "music.h"

static OggVorbis_File vorbisFile;
static vorbis_info * vi;
static FILE * f;
static const size_t buffSize = (8 * 4096);

/**
 * Set decoder parameters for Vorbis.
 *
 * \param	decoder	Structure to store parameters.
 */
void setVorbis(struct decoder_fn * decoder)
{
	decoder->init = &initVorbis;
	decoder->rate = &rateVorbis;
	decoder->channels = &channelVorbis;
	decoder->buffSize = buffSize;
	decoder->decode = &decodeVorbis;
	decoder->exit = &exitVorbis;
}

/**
 * Initialise Vorbis decoder.
 *
 * \param	file	Location of vorbis file to play.
 * \return			0 on success, else failure.
 */
int initVorbis(const char * file)
{
	int err = -1;

	if ((f = fopen(file, "rb")) == NULL)
		goto out;

	if (ov_open(f, &vorbisFile, NULL, 0) < 0)
		goto out;

	if ((vi = ov_info(&vorbisFile, -1)) == NULL)
		goto out;

	err = 0;

out:
	return err;
}

/**
 * Get sampling rate of Vorbis file.
 *
 * \return	Sampling rate.
 */
u32 rateVorbis(void)
{
	return vi->rate;
}

/**
 * Get number of channels of Vorbis file.
 *
 * \return	Number of channels for opened file.
 */
u8 channelVorbis(void)
{
	return vi->channels;
}

/**
 * Decode part of open Vorbis file.
 *
 * \param buffer	Decoded output.
 * \return			Samples read for each channel. 0 for end of file, negative
 *					for error.
 */
u64 decodeVorbis(void * buffer)
{
	return fillVorbisBuffer(buffer);
}

/**
 * Free Vorbis decoder.
 */
void exitVorbis(void)
{
	ov_clear(&vorbisFile);
	fclose(f);
}

/**
 * Decode Vorbis file to fill buffer.
 *
 * \param opusFile		File to decode.
 * \param bufferOut		Pointer to buffer.
 * \return				Samples read per channel.
 */
u64 fillVorbisBuffer(char * bufferOut)
{
	u64 samplesRead = 0;
	int samplesToRead = buffSize;

	while(samplesToRead > 0)
	{
		static int current_section;
		
		int samplesJustRead = ov_read(&vorbisFile, bufferOut, samplesToRead > 4096 ? 4096 : samplesToRead, &current_section);

		if (samplesJustRead < 0)
			return samplesJustRead;
		else if (samplesJustRead == 0) /* End of file reached. */	
			break;

		samplesRead += samplesJustRead;
		samplesToRead -= samplesJustRead;
		bufferOut += samplesJustRead;
	}

	return samplesRead / sizeof(int16_t);
}

/**
 * Checks if the input file is Vorbis.
 *
 * \param in	Input file.
 * \return		0 if Vorbis file, else not or failure.
 */
int isVorbis(const char * in)
{
	FILE * ft = fopen(in, "r");
	OggVorbis_File testvf;
	int err;

	if (ft == NULL)
		return -1;

	err = ov_test(ft, &testvf, NULL, 0);

	ov_clear(&testvf);
	fclose(ft);
	return err;
}