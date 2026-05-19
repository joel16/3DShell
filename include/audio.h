#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <string>

extern bool playing, paused;

typedef struct {
    bool hasMeta = false;
    char title[64];
    char album[64];
    char artist[64];
    char year[64];
    char comment[64];
    char genre[64];
    C2D_Image image;
} AudioMetadata;

extern AudioMetadata metadata;

namespace FLAC {
    int Init(const char *path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace MP3 {
    int Init(const char *path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace OGG {
    int Init(const char *path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace OPUS {
    int Init(const char *path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace WAV {
    int Init(const char *path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace XM {
    int Init(const char *path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace Audio {
    void Init(const char *path);
    bool IsPaused(void);
    void Pause(void);
    void Stop(void);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 GetPositionSeconds(void);
    u64 GetLengthSeconds(void);
    u64 Seek(u64 index);
    void Exit(void);
}
