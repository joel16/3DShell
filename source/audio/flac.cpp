#include <cstring>
#include <FLAC/stream_decoder.h>

#include "audio.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace FLAC {
    typedef struct {
        const FLAC__Frame *frame = nullptr;
        const FLAC__int32 * const *buffer = nullptr;
        FLAC__uint8 channels = 0;
        FLAC__uint32 rate = 0;
        FLAC__uint32 bps = 0;
        FLAC__uint64 position = 0;
        FLAC__uint64 samples = 0;
        FLAC__uint64 totalSamples = 0;
    } FLACInfo;
    
    static FLAC__StreamDecoder *flac = nullptr;
    static FLACInfo info { 0 };
    
    static FLAC__StreamDecoderWriteStatus WriteCB(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data) {
        FLACInfo *info = reinterpret_cast<FLACInfo *>(client_data);

        if (info->totalSamples == 0) {
            Log::Error("This decoder only works for FLAC files that have a total_samples count in STREAMINFO\n");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        if ((info->channels != 2) || (info->bps != 16)) {
            Log::Error("This decoder only supports 16bit stereo streams\n");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        if (frame->header.channels != 2) {
            Log::Error("This frame contains %u channels (should be 2)\n", frame->header.channels);
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        if (buffer[0] == NULL) {
            Log::Error("buffer [0] is NULL\n");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        if (buffer[1] == NULL) {
            Log::Error("buffer [1] is NULL\n");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        
        info->frame = frame;
        info->buffer = buffer;
        info->position = 0;
        info->samples = frame->header.number.sample_number;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }
    
    static void MetadataCB(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *stream, void *client_data) {
        FLACInfo *info = reinterpret_cast<FLACInfo *>(client_data);

        switch (stream->type) {
            case FLAC__METADATA_TYPE_STREAMINFO:
                info->totalSamples = stream->data.stream_info.total_samples;
                info->rate = stream->data.stream_info.sample_rate;
                info->channels = stream->data.stream_info.channels;
                info->bps = stream->data.stream_info.bits_per_sample;
                break;

            case FLAC__METADATA_TYPE_VORBIS_COMMENT:
                for (FLAC__uint32 i = 0; i < stream->data.vorbis_comment.num_comments; i++) {
                    const char *tag = reinterpret_cast<const char *>(stream->data.vorbis_comment.comments[i].entry);
                    metadata.hasMeta = true;
                    
                    if (!strncasecmp("TITLE=", tag, 6)) {
                        Utils::SafeCopy(metadata.title, tag + 6, sizeof(metadata.title));
                    }
                    
                    if (!strncasecmp("ALBUM=", tag, 6)) {
                        Utils::SafeCopy(metadata.album, tag + 6, sizeof(metadata.album));
                    }
                    
                    if (!strncasecmp("ARTIST=", tag, 7)) {
                        Utils::SafeCopy(metadata.artist, tag + 7, sizeof(metadata.artist));
                    }
                    
                    if (!strncasecmp("DATE=", tag, 5)) {
                        Utils::SafeCopy(metadata.year, tag + 5, sizeof(metadata.year));
                    }
                    
                    if (!strncasecmp("COMMENT=", tag, 8)) {
                        Utils::SafeCopy(metadata.comment, tag + 8, sizeof(metadata.comment));
                    }
                    
                    if (!strncasecmp("GENRE=", tag, 6)) {
                        Utils::SafeCopy(metadata.genre, tag + 6, sizeof(metadata.genre));
                    }
                }
                break;

            case FLAC__METADATA_TYPE_PICTURE:
                if (stream->data.picture.type == FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER) {
                    if ((!strcasecmp(stream->data.picture.mime_type, "image/jpeg")) || (!strcasecmp(stream->data.picture.mime_type, "image/jpg"))) {
                        Textures::LoadImageJPEG(stream->data.picture.data, stream->data.picture.data_length, &metadata.image);
                    }
                    else if (!strcasecmp(stream->data.picture.mime_type, "image/png")) {
                        Textures::LoadImagePNG(stream->data.picture.data, stream->data.picture.data_length, &metadata.image);
                    }
                }
                break;

            default:
                break;
        }
    }
    
    static void ErrorCB(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
        Log::Error("FLAC error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
    }
    
    int Init(const char *path) {
        FLAC__StreamDecoderInitStatus ret = FLAC__STREAM_DECODER_INIT_STATUS_OK;

        if ((flac = FLAC__stream_decoder_new()) == nullptr) {
            Log::Error("FLAC__stream_decoder_new failed to initialize: %s", path);
            return -1;
        }

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_STREAMINFO) == false) {
            Log::Error("FLAC__METADATA_TYPE_STREAMINFO response failed\n");
            return -1;
        }

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_SEEKTABLE) == false) {
            Log::Error("FLAC__METADATA_TYPE_SEEKTABLE response failed\n");
        }

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_VORBIS_COMMENT) == false) {
            Log::Error("FLAC__METADATA_TYPE_VORBIS_COMMENT response failed\n");
        }

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_PICTURE) == false) {
            Log::Error("FLAC__METADATA_TYPE_PICTURE response failed\n");
        }
        
        if ((ret = FLAC__stream_decoder_init_file(flac, path, FLAC::WriteCB, FLAC::MetadataCB, FLAC::ErrorCB, &info)) != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
            Log::Error("FLAC__stream_decoder_init_file failed: %s\n", FLAC__StreamDecoderInitStatusString[ret]);
            return ret;
        }
        
        return FLAC__stream_decoder_process_until_end_of_metadata(flac);
    }
    
    u32 GetSampleRate(void) {
        return info.rate;
    }
    
    u8 GetChannels(void) {
        return info.channels;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        unsigned int decoded = 0;
        FLAC__bool ret = false;
        
        if (length <= 0) {
            return;
        }
            
        FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(flac);
        if (state == FLAC__STREAM_DECODER_END_OF_STREAM) {
            playing = false;
        }
        else if (state == FLAC__STREAM_DECODER_ABORTED) {
            return;
        }
            
        while(decoded < length) {
            if ((info.frame == nullptr) || (info.position == info.frame->header.blocksize)) {
                ret = FLAC__stream_decoder_process_single(flac);
                
                if (ret == false) {
                    playing = false;
                    return;
                }
                
                state = FLAC__stream_decoder_get_state(flac);
                
                if ((state == FLAC__STREAM_DECODER_END_OF_STREAM) || (state == FLAC__STREAM_DECODER_ABORTED)) {
                    playing = false;
                    return;
                }
            }
            
            for(; decoded < length && info.position < info.frame->header.blocksize; info.position++, decoded++) {
                // Copy to buffer here; convert from BE to LE
                short *buffer = static_cast<short *>(buf);
                buffer[decoded * 2] = info.buffer[0][info.position];
                buffer[decoded * 2 + 1] = info.buffer[1][info.position];
            }
        }
    }
    
    u64 GetPosition(void) {
        return info.samples;
    }
    
    u64 GetLength(void) {
        return info.totalSamples;
    }
    
    u64 Seek(u64 index) {
        FLAC__uint64 seek_sample = (info.totalSamples * (index / 225.0));
        return FLAC__stream_decoder_seek_absolute(flac, seek_sample);
    }
    
    void Exit(void) {
        info = { 0 };
        FLAC__stream_decoder_finish(flac);
        FLAC__stream_decoder_delete(flac);
    }
}
