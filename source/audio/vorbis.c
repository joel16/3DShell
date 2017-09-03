#include "audio/vorbis.h"

/*static OggVorbis_File	vorbisFile;
static vorbis_info		*vi;
static FILE				*f;
static const size_t		buffSize = 8 * 4096;*/

/**
 * Checks if the input file is Vorbis.
 *
 * \param in	Input file.
 * \return		0 if Vorbis file, else not or failure.
 */
int isVorbis(const char *in)
{
	FILE *ft = fopen(in, "r");
	OggVorbis_File testvf;
	int err;

	if(ft == NULL)
		return -1;

	err = ov_test(ft, &testvf, NULL, 0);

	ov_clear(&testvf);
	fclose(ft);
	return err;
}
