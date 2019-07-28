#ifndef _3DSHELL_DIALOG_H
#define _3DSHELL_DIALOG_H

#include <3ds.h>

void Dialog_DisplayMessage(const char *title, const char *msg_1, const char *msg_2, bool with_bg);
void Dialog_DisplayPrompt(const char *title, const char *msg_1, const char *msg_2, int *selection, bool with_bg);
void Dialog_DisplayProgress(const char *title, const char *message, u32 offset, u32 size);

#endif
