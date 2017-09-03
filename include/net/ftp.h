#ifndef FTP_H
#define FTP_H

/*! Loop status */
typedef enum
{
	LOOP_OFF,		/*!< Looping uninitialized */
	LOOP_CONTINUE, 	/*!< Continue looping */
	LOOP_RESTART,	/*!< Begin looping */
	LOOP_EXIT,		/*!< Terminate looping */
} loop_status_t;

int ftp_init(void);
void ftp_loop(void);
void ftp_exit(void);

#endif