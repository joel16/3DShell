/* Obtained from ctrmus source with permission. */

#include <mpg123.h>

#include "audio.h"

typedef struct 
{
	char title[0x1F];
	char album[0x1F];
	char artist[0x1F];
	char year[0x5];
	char comment[0x1E];
	char genre[0x1F];
} ID3_Tag;

ID3_Tag ID3;

char *id3_pic;

/**
 *Set decoder parameters for MP3.
 *
 *\param decoder Structure to store parameters.
 */
void MP3_SetDecoder(struct decoder_fn *decoder);
