#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/mp3.h"

static uint32_t rate;
static uint8_t channels;

void mp3_thread(void * data);

struct audio * mp3_create(enum channel chan)
{
	struct audio * new_audio = (struct audio *)malloc(sizeof(struct audio));

	if (new_audio == NULL)
		return NULL;

	new_audio->block_pos = 0;
	new_audio->block = false;
	new_audio->channel = chan;

	memset(&(new_audio->waveBuf[0]), 0, sizeof(ndspWaveBuf));
	memset(&(new_audio->waveBuf[1]), 0, sizeof(ndspWaveBuf));

	memset(new_audio->mix, 0, sizeof(new_audio->mix));
	new_audio->mix[0] =
	new_audio->mix[1] = 1.0;

	ndspChnSetInterp(new_audio->channel, NDSP_INTERP_LINEAR);
	ndspChnSetRate(new_audio->channel, 44100);
	ndspChnSetFormat(new_audio->channel, NDSP_FORMAT_STEREO_PCM16);
	ndspChnSetMix(new_audio->channel, new_audio->mix);

	return new_audio;
}

void mp3_load(const char * name, struct audio * audio)
{
	int err = 0;
	int encoding = 0;

	if ((err = mpg123_init()) != MPG123_OK)
		return;

	if ((audio->mpg123 = mpg123_new(NULL, &err)) == NULL)
		return;

	audio->filename = strdup(name);

	if (mpg123_open(audio->mpg123, name) != MPG123_OK || mpg123_getformat(audio->mpg123, (long *) &rate, (int *) &audio->channel, &encoding) != MPG123_OK)
		return;

	mpg123_format_none(audio->mpg123);
	mpg123_format(audio->mpg123, rate, channels, encoding);

	const unsigned long sample_size = 4;
	const unsigned long buffer_size = mpg123_outblock(audio->mpg123) * 16;
	const unsigned long num_samples = (buffer_size / sample_size);

	audio->waveBuf[0].nsamples =
	audio->waveBuf[1].nsamples = num_samples;

	audio->waveBuf[0].status =
	audio->waveBuf[1].status = NDSP_WBUF_DONE;

	audio->waveBuf[0].data_vaddr = linearAlloc(buffer_size);
	audio->waveBuf[1].data_vaddr = linearAlloc(buffer_size);

	LightEvent_Init(&audio->stopEvent, RESET_ONESHOT);
	audio->thread = threadCreate(mp3_thread, audio, 0x1000, 0x3F, -2, false);
}

void mp3_loop(struct audio * audio)
{
	// if (mus_failure <= 0) return;
	size_t done = 0;
	long size = audio->waveBuf[audio->block].nsamples * 4 - audio->block_pos;

	if (audio->waveBuf[audio->block].status == NDSP_WBUF_DONE)
	{
		read:
		//audio->status = ov_read(&audio->vf, (char*)audio->waveBuf[audio->block].data_vaddr + audio->block_pos, size, &audio->section);
		audio->status = mpg123_read(audio->mpg123, (unsigned char *)audio->waveBuf[audio->block].data_vaddr + audio->block_pos, size, &done);

		if (audio->status <= 0)
		{
			mpg123_close(audio->mpg123);

			if (audio->status < 0)
				ndspChnReset(audio->channel);
			else
			{
				// Clarity? Forget that, I don't want to make an new variable. >_<
				audio->status = mpg123_open(audio->mpg123, audio->filename);
				goto read; // TODO: Better way to do this?
			}

		}
		else
		{
			audio->block_pos += audio->status;
			if (audio->status == size)
			{
				audio->block_pos = 0;
				ndspChnWaveBufAdd(audio->channel, &audio->waveBuf[audio->block]);
				audio->block = !audio->block;
			}
		}
	}
}

void mp3_thread(void * data)
{
	struct audio * audio = (struct audio *)data;

	while (!(LightEvent_TryWait(&audio->stopEvent)))
		mp3_loop(audio);

	LightEvent_Clear(&audio->stopEvent);
}
