/* Obtained from ctrmus source with permission. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "audio/wav.h"

static const size_t	buffSize = 16 * 1024;
static Handle pWav;
static char header[45];
static u8 channels;
static u32 bytesRead = 0;
static u64 offset = 0;

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
	decoder->decode = &readWav;
	decoder->exit = &exitWav;
}

/**
 * Initialise WAV playback.
 *
 * \param	file	Location of WAV file to play.
 * \return			0 on success, else failure.
 */
int initWav(const char * file)
{
	if (R_FAILED(fsOpen(&pWav, file, FS_OPEN_READ))) 
		return -1;
	
	/* TODO: No need to read the first number of bytes */
	if (R_FAILED(FSFILE_Read(pWav, &bytesRead, offset, header, 44)))
		return -1;
	
	offset += bytesRead;

	/**
	 * http://www.topherlee.com/software/pcm-tut-wavformat.html and
	 * http://soundfile.sapp.org/doc/WaveFormat/ helped a lot.
	 * format = (header[19]<<8) + (header[20]);
	 * channels = (header[23]<<8) + (header[22]);
	 * sample = (header[27]<<24) + (header[26]<<16) + (header[25]<<8) +
	 *	(header[24]);
	 * byterate = (header[31]<<24) + (header[30]<<16) + (header[29]<<8) +
	 *	(header[28]);
	 * blockalign = (header[33]<<8) + (header[32]);
	 * bitness = (header[35]<<8) + (header[34]);
	 */

	/* TODO: This should be moved to get file type */
	/* Only support 16 bit PCM WAV */
	if (((header[35]<<8) + (header[34])) != 16)
		return -1;

	channels = (header[23]<<8) + (header[22]);

	switch(channels)
	{
		/* Only Mono and Stereo allowed */
		case 1:
		case 2:
			break;

		default:
			return -1;
	}

	return 0;
}

/**
 * Get sampling rate of Wav file.
 *
 * \return	Sampling rate.
 */
uint32_t rateWav(void)
{
	return (header[27]<<24) + (header[26]<<16) + (header[25]<<8) + (header[24]);
}

/**
 * Get number of channels of Wav file.
 *
 * \return	Number of channels for opened file.
 */
u8 channelWav(void)
{
	return channels;
}

/**
 * Read part of open Wav file.
 *
 * \param buffer	Output.
 * \return			Samples read for each channel.
 */
u64 readWav(void * buffer)
{	
	FSFILE_Read(pWav, &bytesRead, offset, buffer, buffSize);
	offset += bytesRead;
	
	return bytesRead / sizeof(u16);
}

/**
 * Free Wav file.
 */
void exitWav(void)
{
	FSFILE_Close(pWav);
	bytesRead = 0;
	offset = 0;
}