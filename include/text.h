#ifndef TEXT_H
#define TEXT_H

#include <ctype.h>

#define MAX_LENGTH_LINE 77

struct fileviewer 
{
	FILE* fp;
	//char **contents;
	struct textviewer *tv;
	char *path;
	int opened;
	long lines;
};

struct textviewer 
{
	int line;
	char *contents;
};

int fileviewer_init(struct fileviewer *fv, const char path[]);
int fileviewer_getline_index(struct fileviewer *fv, int index, char **buf, int *line_number);
int fileviewer_run(struct fileviewer *fv);
int displayText(char * path);

#endif