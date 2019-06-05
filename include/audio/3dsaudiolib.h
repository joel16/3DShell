#ifndef _3D_SHELL_AUDIOLIB_H
#define _3D_SHELL_AUDIOLIB_H

#include <3ds.h>

Result _3dsAudioInit(u8 channels, float rate);
void _3dsAudioEndPre(void);
void _3dsAudioEnd(void);

#endif
