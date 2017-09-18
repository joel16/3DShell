/* Obtained from ctrmus source with permission. */

#include <mpg123.h>

#include "music.h"

typedef struct 
{
	char title[0x1E];
	char album[0x1E];
	char artist[0x1E];
	char year[0x4];
	char comment[0x1E];
	char genre[0x1E];
} ID3v2;

ID3v2 ID3;

char * id3_pic;

/**
 * Set decoder parameters for MP3.
 *
 * \param	decoder Structure to store parameters.
 */
void setMp3(struct decoder_fn * decoder);

/**
 * Initialise MP3 decoder.
 *
 * \param	file	Location of MP3 file to play.
 * \return			0 on success, else failure.
 */
int initMp3(const char * file);

/**
 * Get sampling rate of MP3 file.
 *
 * \return	Sampling rate.
 */
u32 rateMp3(void);

/**
 * Get number of channels of MP3 file.
 *
 * \return	Number of channels for opened file.
 */
u8 channelMp3(void);

/**
 * Decode part of open MP3 file.
 *
 * \param buffer	Decoded output.
 * \return			Samples read for each channel.
 */
u64 decodeMp3(void * buffer);

/**
 * Free MP3 decoder.
 */
void exitMp3(void);