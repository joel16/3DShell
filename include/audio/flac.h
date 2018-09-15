/* Obtained from ctrmus source with permission. */

#include "audio.h"

/**
 *Set decoder parameters for flac.
 *
 *\param decoder Structure to store parameters.
 */
void FLAC_SetDecoder(struct decoder_fn *decoder);

/**
 *Checks if the input file is Flac
 *
 *\param file Input file.
 *\return 0 if Flac file, else not or failure.
 */
int FLAC_Validate(const char *file);
