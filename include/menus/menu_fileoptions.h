#ifndef _3D_SHELL_MENU_FILEOPTIONS_H
#define _3D_SHELL_MENU_FILEOPTIONS_H

#include <3ds.h>

void FileOptions_ResetClipboard(void);
void Menu_DisplayDeleteDialog(void);
void Menu_ControlDeleteDialog(u32 input);
void Menu_DisplayProperties(void);
void Menu_ControlProperties(u32 input);
void Menu_DisplayFileOptions(void);
void Menu_ControlFileOptions(u32 input);

#endif
