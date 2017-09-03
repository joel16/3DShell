#include "audio/opus.h"

/*static OggOpusFile*		opusFile;
static const OpusHead*	opusHead;
static const size_t		buffSize = 32 * 1024;*/

/**
 * Checks if the input file is Opus.
 *
 * \param in	Input file.
 * \return		0 if Opus file, else not or failure.
 */
int isOpus(const char* in)
{
	int err = 0;
	//OggOpusFile* opusTest = op_test_file(in, &err);

	//op_free(opusTest);
	return err;
}
