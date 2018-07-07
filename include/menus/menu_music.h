#ifndef _3D_SHELL_MENU_MUSIC_H
#define _3D_SHELL_MENU_MUSIC_H

#include <3ds.h>

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
	int (* init)(const char *file);
	u32 (* rate)(void);
	u8 (* channels)(void);
	size_t buffSize;
	u64 (* decode)(void*);
	void (* exit)(void);
};

// Returns true if playing.
bool Music_IsPlaying(void);

/**
 * Obtains file type.
 *
 * \param	file	File location.
 * \return			file_types enum or 0 on error and errno set.
 */
enum file_types Music_GetMusicFileType(const char *file);

/**
 * Play an audio file.
 *
 * \param path	File path.
 */
void Music_Player(char *path);

#endif
