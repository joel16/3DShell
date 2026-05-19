#include <cstring>

#include "audiolib.h"

namespace AudioLib {
    static ndspWaveBuf waveBuf[2];
    static s16 *buffer[2] = { nullptr, nullptr };
    static std::size_t bufSize = 0;
    
    static Thread thread;
    static volatile bool runThread = false;
    
    static AudioLib::Callback g_callback = nullptr;
    static void *g_userdata = nullptr;
    static u32 g_samples = 0;
    static u8 g_channels = 0;
    
    static void DecodeSamples(void *buffer, u32 samples) {
        if (g_callback) {
            g_callback(buffer, samples / g_channels, g_userdata);
        }
        else {
            std::memset(buffer, 0, samples * sizeof(s16));
        }
        
        DSP_FlushDataCache(buffer, samples * sizeof(s16));
    }
    
    static void FillBuffers(void) {
        for (int i = 0; i < 2; i++) {
            ndspWaveBuf *buf = &waveBuf[i];
            
            if (buf->status == NDSP_WBUF_DONE || buf->status == NDSP_WBUF_FREE) {
                AudioLib::DecodeSamples(buf->data_pcm16, buf->nsamples * g_channels);
                ndspChnWaveBufAdd(0, buf);
            }
        }
    }
    
    static void AudioThread(void *arg) {
        (void)arg;
        
        while (runThread) {
            AudioLib::FillBuffers();
            svcSleepThread(1 * 1000 * 1000); // 1 ms
        }
    }

    Result Init(u8 channels, float rate, u32 samples) {
        Result ret = 0;
        
        if (R_FAILED(ret = ndspInit())) {
            return ret;
        }
        
        ndspChnReset(0);
        ndspChnWaveBufClear(0);
        ndspSetOutputMode(channels == 2? NDSP_OUTPUT_STEREO : NDSP_OUTPUT_MONO);
        ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
        ndspChnSetRate(0, rate);
        ndspChnSetFormat(0, channels == 2? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);
        
        g_samples = samples;
        g_channels = channels;
        bufSize = sizeof(s16) * channels * samples;
        
        for (int i = 0; i < 2; i++) {
            ndspWaveBuf *buf = &waveBuf[i];
            std::memset(buf, 0, sizeof(ndspWaveBuf));
            
            buffer[i] = (s16 *)linearAlloc(bufSize);
            buf->data_vaddr = buffer[i];
            buf->nsamples = samples;
            AudioLib::DecodeSamples(buf->data_pcm16, buf->nsamples * channels);
            ndspChnWaveBufAdd(0, buf);
        }
        
        return 0;
    }

    void Cleanup(void) {
        AudioLib::Stop();
        ndspChnWaveBufClear(0);
        
        for (int i = 0; i < 2; i++) {
            if (buffer[i]) {
                linearFree(buffer[i]);
                buffer[i] = nullptr;
            }
        }
        
        ndspExit();
    }
    
    void Start(void) {
        if (runThread) {
            return;
        }
        
        runThread = true;
        
        s32 priority;
        svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
        
        thread = threadCreate(AudioLib::AudioThread, nullptr, 16 * 1024, priority - 1, -2, true);
    }
    
    void Stop(void) {
        if (!runThread) {
            return;
        }
        
        runThread = false;
        threadJoin(thread, U64_MAX);
    }
    
    void SetCallback(AudioLib::Callback callback, void *userdata) {
        g_callback = callback;
        g_userdata = userdata;
    }
    
    void SetVolume(float left, float right) {
        float mix[12] = { 0.0f };
        mix[0] = left;
        mix[1] = right;
        ndspChnSetMix(0, mix);
    } 
}
