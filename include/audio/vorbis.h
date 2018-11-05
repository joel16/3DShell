/* Obtained from ctrmus source with permission. */
#include "audio.h"

/**
 *Set decoder parameters for Vorbis.
 *
 *\param decoder Structure to store parameters.
 */
void VORBIS_SetDecoder(struct decoder_fn *decoder);

/**
 *Checks if the input file is Vorbis
 *
 *\param file Input file.
 *\return 0 if Vorbis file, else not or failure.
 */
int VORBIS_Validate(const char *file);
