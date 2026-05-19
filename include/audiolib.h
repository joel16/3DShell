#pragma once

#include <3ds.h>

namespace AudioLib {
    typedef void (*Callback)(void *buffer, u32 samples, void *userdata);

    Result Init(u8 channels, float rate, u32 samples);
    void Cleanup(void);
    void Start(void);
    void Stop(void);
    void SetCallback(AudioLib::Callback callback, void *userdata);
    void SetVolume(float left, float right);
}
