#ifndef MENU_UPDATE_H
#define MENU_UPDATE_H

#include <3ds.h>
#include <setjmp.h>

jmp_buf exitJmp;

void Menu_DisplayUpdate(void);
void Menu_DisplayUpdate2(void);
void Menu_ControlUpdate(u32 input);
void Menu_ControlUpdate2(u32 input);

#endif