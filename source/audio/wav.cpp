#include "audio.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "log.h"

namespace WAV {
    static drwav wav;
    static drwav_uint64 samples = 0;
    
    int Init(const char *path) {
        if (!drwav_init_file(&wav, path, nullptr)) {
            Log::Error("drwav_init_file failed to open file: %s", path);
            return -1;
        }
            
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return wav.sampleRate;
    }
    
    u8 GetChannels(void) {
        return wav.channels;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        samples += drwav_read_pcm_frames_s16(&wav, static_cast<drwav_uint64>(length), static_cast<drwav_int16 *>(buf));
        
        if (samples >= wav.totalPCMFrameCount) {
            playing = false;
        }
    }
    
    u64 GetPosition(void) {
        return samples;
    }
    
    u64 GetLength(void) {
        return wav.totalPCMFrameCount;
    }
    
    u64 Seek(u64 index) {
        drwav_uint64 seek = (wav.totalPCMFrameCount * (index / 225.0));
        
        if (drwav_seek_to_pcm_frame(&wav, seek) == DRWAV_TRUE) {
            samples = seek;
            return samples;
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples = 0;
        drwav_uninit(&wav);
    }
}
