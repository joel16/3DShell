#include "menus/menu_music.h"

/**
 * Set decoder parameters for WAV.
 *
 * \param	decoder Structure to store parameters.
 */
void setWav(struct decoder_fn * decoder);

/**
 * Initialise WAV playback.
 *
 * \param	file	Location of WAV file to play.
 * \return			0 on success, else failure.
 */
int initWav(const char * file);

/**
 * Get sampling rate of Wav file.
 *
 * \return	Sampling rate.
 */
u32 rateWav(void);

/**
 * Get number of channels of Wav file.
 *
 * \return	Number of channels for opened file.
 */
u8 channelWav(void);

/**
 * Read part of open Wav file.
 *
 * \param buffer	Output.
 * \return			Samples read for each channel.
 */
u64 readWav(void * buffer);

/**
 * Free Wav file.
 */
void exitWav(void);