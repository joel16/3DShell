#include "audio.h"

/**
 *Set decoder parameters for WAV.
 *
 *\param decoder Structure to store parameters.
 */
void WAV_SetDecoder(struct decoder_fn *decoder);

/**
 *Checks if the input file is Wav
 *
 *\param file Input file.
 *\return 0 if Wav file, else not or failure.
 */
int WAV_Validate(const char *file);
