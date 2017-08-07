#include "task.h"

static bool task_quit;
static Handle task_pause_event;
static Handle task_suspend_event;
static aptHookCookie cookie;

static bool lcd = true;

static void task_apt_hook(APT_HookType hook, void* param) 
{
	switch(hook) 
	{
		case APTHOOK_ONSUSPEND:
			svcClearEvent(task_suspend_event);
			break;
		case APTHOOK_ONSLEEP:
			if(R_SUCCEEDED(gspLcdInit()))
			{
				GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
				gspLcdExit();
			}
			break;
		case APTHOOK_ONWAKEUP:
			if(R_SUCCEEDED(gspLcdInit()))
			{
				(lcd ? GSPLCD_PowerOnBacklight : GSPLCD_PowerOffBacklight)(GSPLCD_SCREEN_BOTH);
				gspLcdExit();
			}
			break;
		default:
			break;
	}
}

void task_init(void) 
{
	task_quit = false;

	svcCreateEvent(&task_pause_event, RESET_STICKY);

	svcCreateEvent(&task_suspend_event, RESET_STICKY);

	svcSignalEvent(task_pause_event);
	svcSignalEvent(task_suspend_event);

	aptHook(&cookie, task_apt_hook, NULL);
}

void task_exit(void) 
{
	task_quit = true;

	aptUnhook(&cookie);

	if(task_pause_event != 0) 
	{
		svcCloseHandle(task_pause_event);
		task_pause_event = 0;
	}

	if(task_suspend_event != 0) 
	{
		svcCloseHandle(task_suspend_event);
		task_suspend_event = 0;
	}
}