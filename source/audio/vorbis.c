/* Obtained from ctrmus source with permission. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vorbis.h"

static OggVorbis_File vorbisFile;
static vorbis_info *vi;
static FILE *f;
static const size_t buffSize = (8 * 4096);

static int VORBIS_Init(const char *file) {
	int err = -1;

	if ((f = fopen(file, "rb")) == NULL)
		goto out;

	if (ov_open(f, &vorbisFile, NULL, 0) < 0)
		goto out;

	if ((vi = ov_info(&vorbisFile, -1)) == NULL)
		goto out;

	err = 0;

out:
	return err;
}

static u32 VORBIS_GetSampleRate(void) {
	return vi->rate;
}

static u8 VORBIS_GetChannels(void) {
	return vi->channels;
}

static u64 VORBIS_FillBuffer(char *bufferOut) {
	u64 samplesRead = 0;
	int samplesToRead = buffSize;

	while(samplesToRead > 0) {
		static int current_section;
		
		int samplesJustRead = ov_read(&vorbisFile, bufferOut, samplesToRead > 4096 ? 4096 : samplesToRead, &current_section);

		if (samplesJustRead < 0)
			return samplesJustRead;
		else if (samplesJustRead == 0) /* End of file reached. */	
			break;

		samplesRead += samplesJustRead;
		samplesToRead -= samplesJustRead;
		bufferOut += samplesJustRead;
	}

	return samplesRead / sizeof(s16);
}

static u64 VORBIS_Decode(void *buffer) {
	return VORBIS_FillBuffer(buffer);
}

static void VORBIS_Term(void) {
	ov_clear(&vorbisFile);
	fclose(f);
}

void VORBIS_SetDecoder(struct decoder_fn *decoder) {
	decoder->init = &VORBIS_Init;
	decoder->rate = &VORBIS_GetSampleRate;
	decoder->channels = &VORBIS_GetChannels;
	decoder->buffSize = buffSize;
	decoder->decode = &VORBIS_Decode;
	decoder->exit = &VORBIS_Term;
}

int VORBIS_Validate(const char *file) {
	FILE *fd = fopen(file, "r");
	OggVorbis_File testvf;
	
	if (fd == NULL)
		return -1;

	int err = ov_test(fd, &testvf, NULL, 0);

	ov_clear(&testvf);
	fclose(fd);
	return err;
}
