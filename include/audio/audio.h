#ifndef _3D_SHELL_AUDIO_H
#define _3D_SHELL_AUDIO_H

#include <3ds.h>
#include <citro2d.h>

extern bool playing, paused;

typedef struct {
	bool has_meta;
    char title[31];
    char album[31];
    char artist[31];
    char year[5];
    char comment[31];
    char genre[31];
    C2D_Image cover_image;
} Audio_Metadata;

extern Audio_Metadata metadata;

void Audio_Init(const char *path);
bool Audio_IsPaused(void);
void Audio_Pause(void);
void Audio_Stop(void);
u64 Audio_GetPosition(void);
u64 Audio_GetLength(void);
u64 Audio_GetPositionSeconds(void);
u64 Audio_GetLengthSeconds(void);
void Audio_Term(void);

#endif
