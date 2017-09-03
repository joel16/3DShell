#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

/**
 * Checks if the input file is Vorbis.
 *
 * \param in	Input file.
 * \return		0 if Vorbis file, else not or failure.
 */
int isVorbis(const char *in);
