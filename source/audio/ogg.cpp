#include <cstring>
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

#include "audio.h"
#include "log.h"
#include "utils.h"

namespace OGG {
    // struct OggFSFile {
    //     Handle file;
    //     u64 offset;
    // };

    //static OggFSFile *file = nullptr;
    static FILE *file;
    static OggVorbis_File ogg;
    static vorbis_info *info = nullptr;
    static ogg_int64_t samples = 0, totalSamples = 0;
    
    // static std::size_t ogg_callback_read(void *ptr, size_t size, size_t count, void *stream) {
    //     OggFSFile *fs = static_cast<OggFSFile*>(stream);
        
    //     u32 bytesRead = 0;
    //     u32 bytesToRead = size * count;
        
    //     Result ret = FSFILE_Read(fs->file, &bytesRead, fs->offset, ptr, bytesToRead);
        
    //     if (R_FAILED(ret)) {
    //         return 0;
    //     }
        
    //     fs->offset += bytesRead;
    //     return bytesRead / size;
    // }

    // static int ogg_callback_seek(void *stream, ogg_int64_t offset, int whence) {
    //     OggFSFile *fs = static_cast<OggFSFile*>(stream);
        
    //     u64 newPos = 0;
        
    //     switch (whence) {
    //         case SEEK_SET:
    //             newPos = offset;
    //             break;
                
    //         case SEEK_CUR:
    //             newPos = fs->offset + offset;
    //             break;
                
    //         case SEEK_END: {
    //             u64 size = 0;
                
    //             if (R_FAILED(FSFILE_GetSize(fs->file, &size))) {
    //                 return -1;
    //             }
                
    //             newPos = size + offset;
    //             break;
    //         }
            
    //         default:
    //             return -1;
    //     }
        
    //     fs->offset = newPos;
    //     return 0;
    // }
    
    // static int ogg_callback_close(void *stream) {
    //     OggFSFile *fs = static_cast<OggFSFile*>(stream);
    //     FSFILE_Close(fs->file);
    //     delete fs;
    //     return 0;
    // }
    
    // static long ogg_callback_tell(void *stream) {
    //     OggFSFile *fs = static_cast<OggFSFile*>(stream);
    //     return static_cast<long>(fs->offset);
    // }
    
    int Init(const char *path) {
        // file = new OggFSFile{};
        // file->offset = 0;
        
        // Result ret = FSUSER_OpenFileDirectly(&file->file, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
        // if (R_FAILED(ret)) {
        //     delete file;
        //     return -1;
        // }
            
        // ov_callbacks ogg_callbacks;
        // ogg_callbacks.read_func = ogg_callback_read;
        // ogg_callbacks.seek_func = ogg_callback_seek;
        // ogg_callbacks.close_func = ogg_callback_close;
        // ogg_callbacks.tell_func = ogg_callback_tell;
        
        // if (R_FAILED(ov_open_callbacks(&file, &ogg, nullptr, 0, ogg_callbacks))) {
        //     FSFILE_Close(file->file);
        //     delete file;
        //     return -1;
        // }
        if ((file = fopen(path, "rb")) == nullptr) {
            Log::Error("fopen failed to open file: %s", path);
            return -1;
        }

        if (ov_open(file, &ogg, nullptr, 0) < 0) {
            Log::Error("ov_open failed to open file: %s", path);
            return -1;
        }
        
        if ((info = ov_info(&ogg, -1)) == nullptr) {
            // FSFILE_Close(file->file);
            // delete file;
            Log::Error("ov_info failed to get info for file: %s", path);
            return -1;
        }
            
        totalSamples = ov_pcm_total(&ogg, -1);
        vorbis_comment *comment = ov_comment(&ogg, -1);
        
        if (comment != nullptr) {
            metadata.hasMeta = true;
            char *value = nullptr;
            
            if ((value = vorbis_comment_query(comment, const_cast<char*>("title"), 0)) != nullptr) {
                Utils::SafeCopy(metadata.title, value, sizeof(metadata.title));
            }
                
            if ((value = vorbis_comment_query(comment, const_cast<char*>("album"), 0)) != nullptr) {
                Utils::SafeCopy(metadata.album, value, sizeof(metadata.album));
            }
                
            if ((value = vorbis_comment_query(comment, const_cast<char*>("artist"), 0)) != nullptr) {
                Utils::SafeCopy(metadata.artist, value, sizeof(metadata.artist));
            }
                
            if ((value = vorbis_comment_query(comment, const_cast<char*>("year"), 0)) != nullptr) {
                Utils::SafeCopy(metadata.year, value, sizeof(metadata.year));
            }
                
            if ((value = vorbis_comment_query(comment, const_cast<char*>("comment"), 0)) != nullptr) {
                Utils::SafeCopy(metadata.comment, value, sizeof(metadata.comment));
            }
                
            if ((value = vorbis_comment_query(comment, const_cast<char*>("genre"), 0)) != nullptr) {
                Utils::SafeCopy(metadata.genre, value, sizeof(metadata.genre));
            }
        }
        
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return info->rate;
    }
    
    u8 GetChannels(void) {
        return info->channels;
    }
    
    static u64 FillBuffer(char *out, u64 maxSamples) {
        u64 samples = 0;
        int bytesLeft = sizeof(s16) * info->channels * maxSamples;
        
        while (bytesLeft > 0) {
            static int current;
            int bytesRead = ov_read(&ogg, out, bytesLeft, &current);
            
            if (bytesRead < 0) {
                return bytesRead;
            }
            else if (bytesRead == 0) {
                break;
            }
            
            samples += bytesRead / sizeof(s16);
            bytesLeft -= bytesRead;
            out += bytesRead;
        }
        
        return samples;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        u64 maxSamples = length;
        OGG::FillBuffer(static_cast<char *>(buf), maxSamples);
        samples = ov_pcm_tell(&ogg);
        
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
        ogg_int64_t seek = (totalSamples * (index / 225.0));
        
        if (ov_pcm_seek(&ogg, seek) >= 0) {
            samples = seek;
            return samples;
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples = 0;
        ov_clear(&ogg);
    }
}
