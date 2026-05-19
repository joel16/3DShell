#include <mpg123.h>
#include <cstdio>
#include <cstring>

#include "audio.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace MP3 {
    static mpg123_handle *mp3 = nullptr;
    static u64 samples = 0, totalSamples = 0;
    static int channels = 0;
    static long rate = 0;
    
    /* Helper for v1 printing, get these strings their zero byte. */
    void SafePrint(char *out, char *data, std::size_t size) {
        if (size > 30) {
            return;
        }
        
        std::memcpy(out, data, size);
        out[size] = 0;
    }
    
    /* Print out ID3v1 info. */
    void ProcessID3v1(AudioMetadata &tag, mpg123_id3v1 *v1) {
        MP3::SafePrint(tag.title, v1->title, sizeof(v1->title));
        MP3::SafePrint(tag.artist, v1->artist, sizeof(v1->artist));
        MP3::SafePrint(tag.album, v1->album, sizeof(v1->album));
        MP3::SafePrint(tag.year, v1->year, sizeof(v1->year));
        MP3::SafePrint(tag.comment, v1->comment, sizeof(v1->comment));
    }
    
    /* Split up a number of lines separated by \n, \r, both or just zero byte
    and print out each line. */
    void PrintLines(char *out, std::size_t size, mpg123_string *inlines) {
        std::size_t len = 0;
        int hadcr = 0, hadlf = 0;
        char *lines = nullptr, *line  = nullptr;
    
        if (inlines != nullptr && inlines->fill) {
            lines = inlines->p;
            len   = inlines->fill;
        }
        else {
            return;
        }
        
        line = lines;
        for (std::size_t i = 0; i < len; ++i) {
            if (lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0) {
                char save = lines[i]; /* saving, changing, restoring a byte in the data */
                
                if (save == '\n') {
                    ++hadlf;
                }
                if (save == '\r') {
                    ++hadcr;
                }
                if ((hadcr || hadlf) && hadlf % 2 == 0 && hadcr % 2 == 0) {
                    line = nullptr;
                }
                
                if (line) {
                    lines[i] = 0;
                    Utils::SafeCopy(out, line, size);
                    line = nullptr;
                    lines[i] = save;
                }
            }
            else {
                hadlf = hadcr = 0;

                if (line == nullptr) {
                    line = lines + i;
                }
            }
        }
    }
    
    /* Print out the named ID3v2  fields. */
    void ProcessID3v2(AudioMetadata &tag, mpg123_id3v2 *v2) {
        MP3::PrintLines(tag.title, sizeof(tag.title), v2->title);
        MP3::PrintLines(tag.artist, sizeof(tag.artist), v2->artist);
        MP3::PrintLines(tag.album, sizeof(tag.album), v2->album);
        MP3::PrintLines(tag.year, sizeof(tag.year), v2->year);
        MP3::PrintLines(tag.comment, sizeof(tag.comment), v2->comment);
        MP3::PrintLines(tag.genre, sizeof(tag.genre), v2->genre);
    }

    int Init(const char *path) {
        int error = 0;

        mp3 = mpg123_new(nullptr, &error);
        if (error != MPG123_OK) {
            Log::Error("mpg123_new(%s) failed: ", path, mpg123_strerror(mp3));
            return error;
        }

        error = mpg123_param(mp3, MPG123_FLAGS, MPG123_FORCE_SEEKABLE, 0);
        if (error != MPG123_OK) {
            Log::Error("mpg123_param(%s) MPG123_FLAGS failed: ", path, mpg123_strerror(mp3));
            return error;
        }

        error = mpg123_param(mp3, MPG123_FLAGS, MPG123_QUIET, 0);
        if (error != MPG123_OK) {
            Log::Error("mpg123_param(%s) MPG123_FLAGS failed: ", path, mpg123_strerror(mp3));
            return error;
        }

        // Built-in mpg123 support for gapless decoding is disabled for now, since it does not work well with seeking.
        error = mpg123_param(mp3, MPG123_REMOVE_FLAGS, MPG123_GAPLESS, 0);
        if (error != MPG123_OK) {
            Log::Error("mpg123_param(%s) MPG123_REMOVE_FLAGS failed: ", path, mpg123_strerror(mp3));
            return error;
        }
        
        // Tells mpg123 to use a small read-ahead buffer for better MPEG sync; essential for MP3 radio streams.
        error = mpg123_param(mp3, MPG123_ADD_FLAGS, MPG123_SEEKBUFFER, 0);
        if (error != MPG123_OK) {
            Log::Error("mpg123_param(%s) MPG123_ADD_FLAGS failed: ", path, mpg123_strerror(mp3));
            return error;
        }

        // Sets the resync limit to the end of the stream (e.g. don't give up prematurely).
        error = mpg123_param(mp3, MPG123_RESYNC_LIMIT, -1, 0);
        if (error != MPG123_OK) {
            Log::Error("mpg123_param(%s) MPG123_RESYNC_LIMIT failed: ", path, mpg123_strerror(mp3));
            return error;
        }
            
        error = mpg123_param(mp3, MPG123_ADD_FLAGS, MPG123_PICTURE, 0);
        if (error != MPG123_OK) {
            Log::Error("mpg123_param(%s) MPG123_ADD_FLAGS failed: ", path, mpg123_strerror(mp3));
            return error;
        }
            
        error = mpg123_open(mp3, path);
        if (error != MPG123_OK) {
            Log::Error("mpg123_open(%s) failed: ", path, mpg123_strerror(mp3));
            return error;
        }

        mpg123_seek(mp3, 0, SEEK_SET);
        metadata.hasMeta = mpg123_meta_check(mp3);
        
        mpg123_id3v1 *id3v1;
        mpg123_id3v2 *id3v2;

        if (metadata.hasMeta & MPG123_ID3 && mpg123_id3(mp3, &id3v1, &id3v2) == MPG123_OK) {
            if (id3v1) {
                MP3::ProcessID3v1(metadata, id3v1);
            }
            if (id3v2) {
                MP3::ProcessID3v2(metadata, id3v2);
            }
                
            for (std::size_t i = 0; i < id3v2->pictures; ++i) {
                mpg123_picture *pic = &id3v2->picture[i];
                
                // Front cover or other
                if ((pic->type == 3)) {
                    if ((!strcasecmp(pic->mime_type.p, "image/jpeg")) || (!strcasecmp(pic->mime_type.p, "image/jpg"))) {
                        Textures::LoadImageJPEG(pic->data, pic->size, &metadata.image);
                        break;
                    }
                    else if (!strcasecmp(pic->mime_type.p, "image/png")) {
                        Textures::LoadImagePNG(pic->data, pic->size, &metadata.image);
                        break;
                    }
                }
            }
        }

        mpg123_getformat(mp3, &rate, &channels, 0);
        mpg123_format_none(mp3);
        mpg123_format(mp3, rate, channels, MPG123_ENC_SIGNED_16);
        totalSamples = mpg123_length(mp3);
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return rate;
    }
    
    u8 GetChannels(void) {
        return channels;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        int ret = 0;
        std::size_t done = 0;
        
        ret = mpg123_read(mp3, static_cast<unsigned char *>(buf), length * (sizeof(s16) * MP3::GetChannels()), &done);
        samples = mpg123_tell(mp3);

        if ((samples >= totalSamples) || (ret == MPG123_DONE)) {
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
        off_t seek_frame = (totalSamples * (index / 225.0));
        
        if (mpg123_seek(mp3, seek_frame, SEEK_SET) >= 0) {
            samples = seek_frame;
            return samples;
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples = 0;
        mpg123_close(mp3);
        mpg123_delete(mp3);
        mpg123_exit();
    }
}
