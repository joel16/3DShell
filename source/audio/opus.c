/* Obtained from ctrmus source with permission. */
#include <opus/opusfile.h>
#include "audio_opus.h"

static OggOpusFile *opusFile;
static const OpusHead *opusHead;
static const size_t buffSize = 32 * 1024;

static int Opus_Init(const char* file) {
	int err = 0;

	if ((opusFile = op_open_file(file, &err)) == NULL)
		goto out;

	if ((err = op_current_link(opusFile)) < 0)
		goto out;

	opusHead = op_head(opusFile, err);

out:
	return err;
}

static u32 Opus_GetSampleRate(void) {
	return 48000;
}

static u8 Opus_GetChannels(void) {
	return 2;
}

static u64 Opus_FillBuffer(void *buffer) {
	u64 samplesRead = 0;
	int samplesToRead = buffSize;
	s16 *buf = buffer;

	while(samplesToRead > 0) {
		int samplesJustRead = op_read_stereo(opusFile, buf, samplesToRead > 120*48*2 ? 120*48*2 : samplesToRead);

		if(samplesJustRead < 0)
			return samplesJustRead;
		else if(samplesJustRead == 0) {
			/* End of file reached. */
			break;
		}

		samplesRead += samplesJustRead * 2;
		samplesToRead -= samplesJustRead * 2;
		buf += samplesJustRead * 2;
	}

	return samplesRead;
}

static void Opus_Term(void) {
	op_free(opusFile);
}

void Opus_SetDecoder(struct decoder_fn *decoder) {
	decoder->init = &Opus_Init;
	decoder->rate = &Opus_GetSampleRate;
	decoder->channels = &Opus_GetChannels;
	decoder->buffSize = buffSize;
	decoder->decode = &Opus_FillBuffer;
	decoder->exit = &Opus_Term;
}

int Opus_Validate(const char *file) {
	int err = 0;
	OggOpusFile* opusTest = op_test_file(file, &err);

	op_free(opusTest);
	return err;
}
