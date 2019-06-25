#include <string.h>
#include <stdlib.h>

#include "3dsaudiolib.h"
#include "audio.h"

static ndspWaveBuf wave_buf[2];
static s16 *audio_buffer[2] = {0};
static size_t buf_size = 0;
Thread thread;
Handle handle;
volatile bool run_thread = true;

static void _3dsAudioDecodeSamples(s16 *buf, u32 size) {
	Audio_Callback(NULL, buf, size);
	DSP_FlushDataCache(buf, size);
}

Result _3dsAudioInit(u8 channels, float rate, u32 samples) {
	Result ret = 0;

	if (R_FAILED(ret = ndspInit()))
		return ret;
	
	ndspChnReset(0);
	ndspChnWaveBufClear(0);
	ndspSetOutputMode(channels == 2? NDSP_OUTPUT_STEREO : NDSP_OUTPUT_MONO);
	ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(0, rate);
	ndspChnSetFormat(0, channels == 2? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

	buf_size = sizeof(s16) * channels * samples;

	for (int i = 0; i < 2; i++) {
		ndspWaveBuf *buf = &wave_buf[i];
		memset(buf, 0, sizeof(ndspWaveBuf));

		audio_buffer[i] = (s16 *)linearAlloc(buf_size);
		buf->data_vaddr = audio_buffer[i];
		buf->nsamples = samples;
		_3dsAudioDecodeSamples(buf->data_pcm16, buf_size);
		ndspChnWaveBufAdd(0, buf);
	}

	return 0;
}

void _3dsAudioEnd(void) {
	ndspChnWaveBufClear(0);

	for (int i = 0; i < 2; i++) {
		if (audio_buffer[i])
			linearFree(audio_buffer[i]);
	}

	ndspExit();
}

static void _3dsAudioFillBuffers(void) {
	for (int i = 0; i < 2; i++) {
		ndspWaveBuf *buf = &wave_buf[i];
		if (buf->status == NDSP_WBUF_DONE || buf->status == NDSP_WBUF_FREE) {
			_3dsAudioDecodeSamples(buf->data_pcm16, buf_size);
			ndspChnWaveBufAdd(0, buf);
		}
	}
}

void _3dsAudioThread(void *arg) {
	while(run_thread) {
		svcWaitSynchronization(handle, U64_MAX);
		svcClearEvent(handle);
		_3dsAudioFillBuffers();
	}
}

void _3dsAudioCreateThread(void) {
	run_thread = true;
	svcCreateEvent(&handle, 0);
	
	s32 priority = 0;
	svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
	thread = threadCreate(_3dsAudioThread, NULL, 16 * 1024, priority - 1, -2, true);
}

void _3dsAudioRunThread(void) {
	svcSignalEvent(handle);
}

void _3dsAudioExitThread(void) {
	run_thread = false;
	svcSignalEvent(handle);
	threadJoin(thread, U64_MAX);
	svcCloseHandle(handle);
}
