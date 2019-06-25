#ifndef _3D_SHELL_AUDIOLIB_H
#define _3D_SHELL_AUDIOLIB_H

#include <3ds.h>

Result _3dsAudioInit(u8 channels, float rate, u32 samples);
void _3dsAudioEnd(void);
void _3dsAudioCreateThread(void);
void _3dsAudioRunThread(void);
void _3dsAudioExitThread(void);

#endif
