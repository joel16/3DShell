#include <xmp.h>

#include "audio.h"
#include "log.h"
#include "utils.h"

namespace XM {
    static xmp_context xmp;
    static struct xmp_frame_info frameInfo;
    static struct xmp_module_info moduleInfo;
    static u64 samples = 0, totalSamples = 0;
    
    int Init(const char *path) {
        xmp = xmp_create_context();
        if (xmp_load_module(xmp, const_cast<char *>(path)) < 0) {
            Log::Error("xmp_load_module failed to open file: %s", path);
            return -1;
        }
            
        xmp_start_player(xmp, 44100, 0);
        xmp_get_frame_info(xmp, &frameInfo);
        totalSamples = (frameInfo.total_time * 44.1);
        
        xmp_get_module_info(xmp, &moduleInfo);
        if (moduleInfo.mod->name[0] != '\0') {
            metadata.hasMeta = true;
            Utils::SafeCopy(metadata.title, moduleInfo.mod->name, sizeof(metadata.title));
        }
        
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return 44100;
    }
    
    u8 GetChannels(void) {
        return 2;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        xmp_play_buffer(xmp, buf, static_cast<int>(length) * (sizeof(s16) * 2), 0);
        samples += length;
        
        if (samples >= totalSamples) {
            playing = false;
        }
    }
    
    u64 GetPosition(void) {
        return samples;
    }
    
    u64 GetLength(void) {
        return totalSamples;
    }
    
    u64 Seek(u64 index) {
        int seek = (totalSamples * (index / 225.0));
        
        if (xmp_seek_time(xmp, (seek / 44.1)) >= 0) {
            samples = seek;
            return samples;
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples = 0;
        xmp_end_player(xmp);
        xmp_release_module(xmp);
        xmp_free_context(xmp);
    }
}
