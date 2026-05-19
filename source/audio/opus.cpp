#include <opusfile.h>

#include "audio.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace OPUS {
    static OggOpusFile *opus;
    static ogg_int64_t samples = 0, totalSamples = 0;
    
    int Init(const char *path) {
        int error = 0;
        if ((opus = op_open_file(path, &error)) == nullptr) {
            Log::Error("op_open_file failed to open: %s", path);
            return -1;
        }
            
        if ((error = op_current_link(opus)) < 0) {
            Log::Error("op_current_link failed to link: %s", path);
            return -1;
        }
            
        totalSamples = op_pcm_total(opus, -1);
        const OpusTags *tags = op_tags(opus, 0);

        if (tags != nullptr) {
            metadata.hasMeta = true;
        }

        if (opus_tags_query_count(tags, "title") > 0) {
            Utils::SafeCopy(metadata.title, opus_tags_query(tags, "title", 0), sizeof(metadata.title));
        }
        
        if (opus_tags_query_count(tags, "album") > 0) {
            Utils::SafeCopy(metadata.album, opus_tags_query(tags, "album", 0), sizeof(metadata.album));
        }
        
        if (opus_tags_query_count(tags, "artist") > 0) {
            Utils::SafeCopy(metadata.artist, opus_tags_query(tags, "artist", 0), sizeof(metadata.artist));
        }
        
        if (opus_tags_query_count(tags, "date") > 0) {
            Utils::SafeCopy(metadata.year, opus_tags_query(tags, "date", 0), sizeof(metadata.year));
        }
        
        if (opus_tags_query_count(tags, "comment") > 0) {
            Utils::SafeCopy(metadata.comment, opus_tags_query(tags, "comment", 0), sizeof(metadata.comment));
        }
        
        if (opus_tags_query_count(tags, "genre") > 0) {
            Utils::SafeCopy(metadata.genre, opus_tags_query(tags, "genre", 0), sizeof(metadata.genre));
        }
        
        if (opus_tags_query_count(tags, "METADATA_BLOCK_PICTURE") > 0) {
            OpusPictureTag picture = { 0 };
            opus_picture_tag_init(&picture);
            const char *metadataBlock = opus_tags_query(tags, "METADATA_BLOCK_PICTURE", 0);
            
            int error = opus_picture_tag_parse(&picture, metadataBlock);
            if (error == 0) {
                if (picture.type == 3) {
                    if (picture.format == OP_PIC_FORMAT_JPEG) {
                        Textures::LoadImageJPEG(picture.data, picture.data_length, &metadata.image);
                    }
                    else if (picture.format == OP_PIC_FORMAT_PNG) {
                        Textures::LoadImagePNG(picture.data, picture.data_length, &metadata.image);
                    }
                }
            }
            
            opus_picture_tag_clear(&picture);
        }
        
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return 48000;
    }
    
    u8 GetChannels(void) {
        return 2;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        int samplesRead = op_read_stereo(opus, static_cast<opus_int16 *>(buf), length);
        if (samplesRead) {
            samples = op_pcm_tell(opus);
        }
            
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
        if (op_seekable(opus) >= 0) {
            ogg_int64_t seek = (totalSamples * (index / 225.0));
            
            if (op_pcm_seek(opus, seek) >= 0) {
                samples = seek;
                return samples;
            }
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples = 0;

        if (opus) {
            op_free(opus);
            opus = nullptr;
        }
    }
}
