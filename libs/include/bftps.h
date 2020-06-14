#ifndef BFTPS_H
#define BFTPS_H

#include <limits.h>
#define MAX_PATH PATH_MAX 

#ifdef __cplusplus
extern "C" {
#endif   
    typedef enum {
        FILE_SENDING,
        FILE_RECEIVING,
    } bftps_file_transfer_mode_t;
    
    typedef struct _bftps_file_transfer_t {
        bftps_file_transfer_mode_t mode;
        unsigned int fileSize; // This is only valid for sending files
        unsigned int filePosition;
        struct _bftps_file_transfer_t* next;
        char name[MAX_PATH];
    } bftps_file_transfer_t;
    
    extern int bftps_start(); 
    extern int bftps_stop();
    extern const char* bftps_name();

    extern const bftps_file_transfer_t* bftps_file_transfer_retrieve();
    extern void bftps_file_transfer_cleanup(const bftps_file_transfer_t* file_transfer);

#ifdef __cplusplus
}
#endif

#endif /* BFTPS_H */

