#ifndef MUSIC_H
#define MUSIC_H

#include <3ds.h>

#define TEXTURE_MUSIC_TOP_BG 52
#define TEXTURE_MUSIC_BOTTOM_BG 53
#define TEXTURE_MUSIC_PLAY 54
#define TEXTURE_MUSIC_PAUSE 55

enum file_types
{
	FILE_TYPE_ERROR = 0,
	FILE_TYPE_WAV,
	FILE_TYPE_FLAC,
	FILE_TYPE_VORBIS,
	FILE_TYPE_MP3
};

struct decoder_fn
{
	int (* init)(const char* file);
	u32 (* rate)(void);
	u8 (* channels)(void);
	size_t buffSize;
	u64 (* decode)(void*);
	void (* exit)(void);
};

/**
 * Obtains file type.
 *
 * \param	file	File location.
 * \return			file_types enum or 0 on error and errno set.
 */
enum file_types getMusicFileType(const char *file);

/**
 * Play an audio file.
 *
 * \param path	File path.
 */
void musicPlayer(char * path);

#endif
