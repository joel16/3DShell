/* Obtained from ctrmus source with permission. */

#include "audio.h"

/**
 *Set decoder parameters for flac.
 *
 *\param	decoder Structure to store parameters.
 */
void setFlac(struct decoder_fn *decoder);

/**
 *Initialise Flac decoder.
 *
 *\param	file	Location of flac file to play.
 *\return			0 on success, else failure.
 */
int initFlac(const char *file);

/**
 *Get sampling rate of Flac file.
 *
 *\return	Sampling rate.
 */
u32 rateFlac(void);

/**
 *Get number of channels of Flac file.
 *
 *\return	Number of channels for opened file.
 */
u8 channelFlac(void);

/**
 *Decode part of open Flac file.
 *
 *\param buffer	Decoded output.
 *\return			Samples read for each channel.
 */
u64 decodeFlac(void *buffer);

/**
 *Free Flac decoder.
 */
void exitFlac(void);

/**
 *Checks if the input file is Flac
 *
 *\param in	Input file.
 *\return		0 if Flac file, else not or failure.
 */
int isFlac(const char *in);