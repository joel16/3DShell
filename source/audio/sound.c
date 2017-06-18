/*
	Code from undertale_techdemo by  Xavyrr and kitling. 
*/

#include "sound.h"

void ogg_thread(void *data);

bool isPlaying = false;

void audio_init() 
{
	ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
}

struct sound* sound_create(enum channel chan) 
{
	struct sound *new_sound = (struct sound*)malloc(sizeof(struct sound));
	
	if (new_sound == NULL) 
		return NULL;

    new_sound->block_pos = 0;
    new_sound->block = false;
    new_sound->channel = chan;

    memset(&(new_sound->waveBuf[0]), 0, sizeof(ndspWaveBuf));
    memset(&(new_sound->waveBuf[1]), 0, sizeof(ndspWaveBuf));

    memset(new_sound->mix, 0, sizeof(new_sound->mix));
    new_sound->mix[0] =
    new_sound->mix[1] = 1.0;

    ndspChnSetInterp(new_sound->channel, NDSP_INTERP_LINEAR);
    ndspChnSetRate(new_sound->channel, 44100);
    ndspChnSetFormat(new_sound->channel, NDSP_FORMAT_STEREO_PCM16);
    ndspChnSetMix(new_sound->channel, new_sound->mix);

    return new_sound;
}

// Audio load/play
void audio_load_ogg(const char *name, struct sound *sound) 
{
	const unsigned long sample_size = 4;
	const unsigned long buffer_size = 40960;
	const unsigned long num_samples = buffer_size / sample_size;

	/// Copied from ivorbisfile_example.c
	sound->filename = strdup(name);
	FILE *mus = fopen(name, "rb");
	
	if ((sound->status = ov_open(mus, &sound->vf, NULL, 0))) 
		return;

	sound->waveBuf[0].nsamples =
	sound->waveBuf[1].nsamples = num_samples;

	sound->waveBuf[0].status =
	sound->waveBuf[1].status = NDSP_WBUF_DONE;

	sound->waveBuf[0].data_vaddr = linearAlloc(buffer_size);
	sound->waveBuf[1].data_vaddr = linearAlloc(buffer_size);

	LightEvent_Init(&sound->stopEvent, RESET_ONESHOT);
	sound->thread = threadCreate(ogg_thread, sound, 0x1000, 0x3F, -2, false);
	
	isPlaying = true;
}

void ogg_loop(struct sound *sound) 
{
	// if (mus_failure <= 0) return;

	long size = sound->waveBuf[sound->block].nsamples * 4 - sound->block_pos;

	if (sound->waveBuf[sound->block].status == NDSP_WBUF_DONE)
	{
		read:
		sound->status = ov_read(&sound->vf, (char*)sound->waveBuf[sound->block].data_vaddr + sound->block_pos, size, &sound->section);

        if (sound->status <= 0) 
		{
            ov_clear(&sound->vf);

            if (sound->status < 0) 
				ndspChnReset(sound->channel);
            else 
			{
                // Clarity? Forget that, I don't want to make an new variable. >_<
                sound->status = ov_open(fopen(sound->filename, "rb"), &sound->vf, NULL, 0);
                goto read; // TODO: Better way to do this?
            }

        } 
		else 
		{
            sound->block_pos += sound->status;
            if (sound->status == size) 
			{
                sound->block_pos = 0;
                ndspChnWaveBufAdd(sound->channel, &sound->waveBuf[sound->block]);
                sound->block = !sound->block;
            }
        }
    }
}

void ogg_thread(void *data) 
{
    struct sound *sound = (struct sound*)data;
    while ((isPlaying) && !(LightEvent_TryWait(&sound->stopEvent))) 
		ogg_loop(sound);
    
	LightEvent_Clear(&sound->stopEvent);
}

void sound_stop(struct sound *sound) 
{
	LightEvent_Signal(&sound->stopEvent);
	threadJoin(sound->thread, U64_MAX);
	threadFree(sound->thread);

	ndspChnReset(sound->channel);
	GSPGPU_FlushDataCache(sound->waveBuf[0].data_vaddr, sound->waveBuf[0].nsamples * 4);
	GSPGPU_FlushDataCache(sound->waveBuf[1].data_vaddr, sound->waveBuf[1].nsamples * 4);
	linearFree((void*)sound->waveBuf[0].data_vaddr);
	linearFree((void*)sound->waveBuf[1].data_vaddr);
	// memset (buffer, 0, size);
	
	isPlaying = false;
}

void audio_stop(void) 
{
    ndspExit();
}