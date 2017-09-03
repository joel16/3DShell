/*
	Modified code adapted from undertale_techdemo by  Xavyrr and kitling.
*/
#include <string.h>

#include "audio/ogg.h"

void ogg_thread(void * data);

struct audio * ogg_create(enum channel chan)
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

void ogg_load(const char * name, struct audio * audio)
{
	const unsigned long sample_size = 4;
	const unsigned long buffer_size = 40960;
	const unsigned long num_samples = (buffer_size / sample_size);

	/// Copied from ivorbisfile_example.c
	audio->filename = strdup(name);
	FILE * mus = fopen(name, "rb");

	if ((audio->status = ov_open(mus, &audio->vf, NULL, 0)) < 0)
		return;

	audio->waveBuf[0].nsamples =
	audio->waveBuf[1].nsamples = num_samples;

	audio->waveBuf[0].status =
	audio->waveBuf[1].status = NDSP_WBUF_DONE;

	audio->waveBuf[0].data_vaddr = linearAlloc(buffer_size);
	audio->waveBuf[1].data_vaddr = linearAlloc(buffer_size);

	LightEvent_Init(&audio->stopEvent, RESET_ONESHOT);
	audio->thread = threadCreate(ogg_thread, audio, 0x1000, 0x3F, -2, false);
}

void ogg_loop(struct audio * audio)
{
	// if (mus_failure <= 0) return;

	long size = audio->waveBuf[audio->block].nsamples * 4 - audio->block_pos;

	if (audio->waveBuf[audio->block].status == NDSP_WBUF_DONE)
	{
		read:
		audio->status = ov_read(&audio->vf, (char*)audio->waveBuf[audio->block].data_vaddr + audio->block_pos, size, &audio->section);

		if (audio->status <= 0)
		{
			ov_clear(&audio->vf);

			if (audio->status < 0)
				ndspChnReset(audio->channel);
			else
			{
				// Clarity? Forget that, I don't want to make an new variable. >_<
				audio->status = ov_open(fopen(audio->filename, "rb"), &audio->vf, NULL, 0);
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

void ogg_thread(void * data)
{
	struct audio * audio = (struct audio *)data;

	while (!(LightEvent_TryWait(&audio->stopEvent)))
		ogg_loop(audio);

	LightEvent_Clear(&audio->stopEvent);
}
