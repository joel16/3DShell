#pragma once

/*! Loop status */
typedef enum
{
	LOOP_OFF,		/*!< Looping uninitialized */
	LOOP_CONTINUE, 	/*!< Continue looping */
	LOOP_RESTART,	/*!< Begin looping */
	LOOP_EXIT,		/*!< Terminate looping */
} loop_status_t;

int           ftp_init(void);
loop_status_t ftp_loop(void);
void          ftp_exit(void);