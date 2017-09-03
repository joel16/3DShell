#include <string.h>
#include <stdlib.h>

#include "audio/audio.h"

bool audio_isPlaying(struct audio * audio)
{
	return ndspChnIsPlaying(audio->channel);
}

bool audio_isPaused(struct audio * audio)
{
	return ndspChnIsPaused(audio->channel);
}

bool audio_togglePlayback(struct audio * audio)
{
	bool paused = ndspChnIsPaused(audio->channel);
	ndspChnSetPaused(audio->channel, !paused);
	return !paused;
}

void audio_stop(struct audio * audio)
{
	LightEvent_Signal(&audio->stopEvent);
	threadJoin(audio->thread, U64_MAX);
	threadFree(audio->thread);

	ndspChnReset(audio->channel);

	GSPGPU_FlushDataCache(audio->waveBuf[0].data_vaddr, (audio->waveBuf[0].nsamples * 4));
	GSPGPU_FlushDataCache(audio->waveBuf[1].data_vaddr, (audio->waveBuf[1].nsamples * 4));
	linearFree((void*)audio->waveBuf[0].data_vaddr);
	linearFree((void*)audio->waveBuf[1].data_vaddr);
	// memset (buffer, 0, size);
}
