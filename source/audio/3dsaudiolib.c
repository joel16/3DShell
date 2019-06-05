#include <string.h>
#include <stdlib.h>

#include "3dsaudiolib.h"
#include "audio.h"

#define AUDIO_BUFSIZE 960
#define NUM_BUFFERS   2

static ndspWaveBuf wave_buf[NUM_BUFFERS];
static u32 *audio_buffer = NULL;
static bool fill_block = false;
static int buf_size = 0;

static void ndsp_callback(void *userdata) {
	if (wave_buf[fill_block].status == NDSP_WBUF_DONE) {
		Audio_Callback(userdata, wave_buf[fill_block].data_pcm16, buf_size);
		DSP_FlushDataCache(wave_buf[fill_block].data_pcm8, AUDIO_BUFSIZE * 4);
		wave_buf[fill_block].status = NDSP_WBUF_FREE;
		ndspChnWaveBufAdd(0, &wave_buf[fill_block]);
		fill_block = !fill_block;
	}
}

Result _3dsAudioInit(u8 channels, float rate) {
	Result ret = 0;
	
	buf_size = sizeof(s16) * 2 * AUDIO_BUFSIZE;

	if (R_FAILED(ret = ndspInit()))
		return ret;

	audio_buffer = (u32 *)linearAlloc(buf_size * 2);
	memset(audio_buffer, 0, buf_size * 2);
	ndspSetCallback(ndsp_callback, audio_buffer);

	memset(&wave_buf[0], 0, sizeof(ndspWaveBuf));
	memset(&wave_buf[1], 0, sizeof(ndspWaveBuf));

	ndspChnReset(0);
	ndspChnWaveBufClear(0);
	ndspSetOutputMode(channels == 2? NDSP_OUTPUT_STEREO : NDSP_OUTPUT_MONO);
	ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(0, rate);
	ndspChnSetFormat(0, channels == 2? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

	wave_buf[0].data_vaddr = &audio_buffer[0];
	wave_buf[0].nsamples = AUDIO_BUFSIZE;
	ndspChnWaveBufAdd(0, &wave_buf[0]);

	wave_buf[1].data_vaddr = &audio_buffer[buf_size];
	wave_buf[1].nsamples = AUDIO_BUFSIZE;
	ndspChnWaveBufAdd(0, &wave_buf[1]);

	return 0;
}

void _3dsAudioEndPre(void) {
	return;
}

void _3dsAudioEnd(void) {
	linearFree(audio_buffer);
	ndspExit();
}
