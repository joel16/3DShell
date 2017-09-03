#define DR_FLAC_IMPLEMENTATION
#include <dr_libs/dr_flac.h>

#include "audio/flac.h"

/*static drflac*		pFlac;
static const size_t	buffSize = 16 * 1024;*/

/**
 * Checks if the input file is Flac
 *
 * \param in	Input file.
 * \return		0 if Flac file, else not or failure.
 */
int isFlac(const char* in)
{
	int err = -1;
	drflac* pFlac = drflac_open_file(in);

	if(pFlac != NULL)
		err = 0;

	drflac_close(pFlac);
	return err;
}
