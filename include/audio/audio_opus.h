/* Obtained from ctrmus source with permission. */
#include "audio.h"

/**
 * Set decoder parameters for Opus.
 *
 * \param decoder Structure to store parameters.
 */
void Opus_SetDecoder(struct decoder_fn *decoder);

/**
 * Checks if the input file is Opus.
 *
 * \param in Input file.
 * \return 0 if Opus file, else not or failure.
 */
int Opus_Validate(const char *file);
