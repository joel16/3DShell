#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

/*
*	Copy Flags
*/
#define COPY_FOLDER_RECURSIVE 2
#define COPY_DELETE_ON_FINISH 1
#define COPY_KEEP_ON_FINISH 0
#define NOTHING_TO_COPY -1


/*
*	Copy Move Origin
*/
char copysource[1024];

extern int copymode;

void newFolder();
int renameFile(void);
int delete(void);
void copy(int flag);
int copy_file(char * a, char * b);
int copy_folder_recursive(char * a, char * b);
int paste(void);
void installDirectories();
void saveLastDirectory();

#endif