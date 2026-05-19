#include <cstring>

#include "audio.h"
#include "audiolib.h"
#include "fs.h"
#include "textures.h"

AudioMetadata metadata = { 0 };
bool playing = true, paused = false;

namespace Audio {
    enum AudioFileType {
        FILE_TYPE_NONE,
        FILE_TYPE_FLAC,
        FILE_TYPE_MP3,
        FILE_TYPE_OGG,
        FILE_TYPE_OPUS,
        FILE_TYPE_WAV,
        FILE_TYPE_XM
    };

    typedef struct {
        int (* init)(const char *path);
        u32 (* rate)(void);
        u8 (* channels)(void);
        void (* decode)(void *buffer, unsigned int samples, void *userdata);
        u64 (* position)(void);
        u64 (* length)(void);
        u64 (* seek)(u64 index);
        void (* term)(void);
    } Decoder;
    
    static enum AudioFileType fileType = FILE_TYPE_NONE;
    static Decoder decoder = { 0 };

    static void Decode(void *buffer, u32 samples, void *userdata) {
        if ((!playing) || (paused)) {
            std::memset(buffer, 0, samples * (* decoder.channels)() * sizeof(s16));
        } 
        else {
            (*decoder.decode)(buffer, samples, userdata);
        }
    }

    void Init(const char *path) {
        playing = true;
        paused = false;
        const char *ext = FS::GetFileExt(path);

        if (strncasecmp(ext, "flac", 4) == 0) {
            fileType = FILE_TYPE_FLAC;
        }
        else if (strncasecmp(ext, "mp3", 3) == 0) {
            fileType = FILE_TYPE_MP3;
        }
        else if (strncasecmp(ext, "ogg", 3) == 0) {
            fileType = FILE_TYPE_OGG;
        }
        else if (strncasecmp(ext, "opus", 4) == 0) {
            fileType = FILE_TYPE_OPUS;
        }
        else if (strncasecmp(ext, "wav", 3) == 0) {
            fileType = FILE_TYPE_WAV;
        }
        else if (strncasecmp(ext, "xm", 2) == 0) {
            fileType = FILE_TYPE_XM;
        }

        switch(fileType) {
            case FILE_TYPE_FLAC:
                decoder.init = FLAC::Init;
                decoder.rate = FLAC::GetSampleRate;
                decoder.channels = FLAC::GetChannels;
                decoder.decode = FLAC::Decode;
                decoder.position = FLAC::GetPosition;
                decoder.length = FLAC::GetLength;
                decoder.seek = FLAC::Seek;
                decoder.term = FLAC::Exit;
                break;
            
            case FILE_TYPE_MP3:
                decoder.init = MP3::Init;
                decoder.rate = MP3::GetSampleRate;
                decoder.channels = MP3::GetChannels;
                decoder.decode = MP3::Decode;
                decoder.position = MP3::GetPosition;
                decoder.length = MP3::GetLength;
                decoder.seek = MP3::Seek;
                decoder.term = MP3::Exit;
                break;

            case FILE_TYPE_OGG:
                decoder.init = OGG::Init;
                decoder.rate = OGG::GetSampleRate;
                decoder.channels = OGG::GetChannels;
                decoder.decode = OGG::Decode;
                decoder.position = OGG::GetPosition;
                decoder.length = OGG::GetLength;
                decoder.seek = OGG::Seek;
                decoder.term = OGG::Exit;
                break;

            case FILE_TYPE_OPUS:
                decoder.init = OPUS::Init;
                decoder.rate = OPUS::GetSampleRate;
                decoder.channels = OPUS::GetChannels;
                decoder.decode = OPUS::Decode;
                decoder.position = OPUS::GetPosition;
                decoder.length = OPUS::GetLength;
                decoder.seek = OPUS::Seek;
                decoder.term = OPUS::Exit;
                break;
            
            case FILE_TYPE_WAV:
                decoder.init = WAV::Init;
                decoder.rate = WAV::GetSampleRate;
                decoder.channels = WAV::GetChannels;
                decoder.decode = WAV::Decode;
                decoder.position = WAV::GetPosition;
                decoder.length = WAV::GetLength;
                decoder.seek = WAV::Seek;
                decoder.term = WAV::Exit;
                break;

            case FILE_TYPE_XM:
                decoder.init = XM::Init;
                decoder.rate = XM::GetSampleRate;
                decoder.channels = XM::GetChannels;
                decoder.decode = XM::Decode;
                decoder.position = XM::GetPosition;
                decoder.length = XM::GetLength;
                decoder.seek = XM::Seek;
                decoder.term = XM::Exit;
                break;

            default:
                break;
        }
        
        (* decoder.init)(path);
        AudioLib::Init((* decoder.channels)(), (* decoder.rate)(), 4096);
        AudioLib::SetCallback(Audio::Decode, nullptr);
        AudioLib::Start();
    }
    
    bool IsPaused(void) {
        return paused;
    }
    
    void Pause(void) {
        paused = !paused;
    }
    
    void Stop(void) {
        playing = !playing;
    }
    
    u64 GetPosition(void) {
        return (* decoder.position)();
    }
    
    u64 GetLength(void) {
        return (* decoder.length)();
    }
    
    u64 GetPositionSeconds(void) {
        return (Audio::GetPosition() / (* decoder.rate)());
    }
    
    u64 GetLengthSeconds(void) {
        return (Audio::GetLength() / (* decoder.rate)());
    }
    
    u64 Seek(u64 index) {
        return (* decoder.seek)(index);
    }
    
    void Exit(void) {
        playing = true;
        paused = false;
        
        AudioLib::Stop();
        AudioLib::Cleanup();
        (* decoder.term)();

        Textures::Free(&metadata.image);
        metadata = { 0 };
        decoder =  { 0 };
    }
}
