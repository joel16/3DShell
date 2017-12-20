#ifndef MENU_MUSIC_H
#define MENU_MUSIC_H

#include <3ds.h>

#define TEXTURE_MUSIC_TOP_BG 49
#define TEXTURE_MUSIC_BOTTOM_BG 50
#define TEXTURE_MUSIC_PLAY 51
#define TEXTURE_MUSIC_PAUSE 52
#define TEXTURE_MUSIC_ICON 53

enum file_types
{
	FILE_TYPE_ERROR = 0,
	FILE_TYPE_BCSTM,
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
void menu_musicPlayer(char * path);

#endif
