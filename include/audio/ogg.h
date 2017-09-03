#ifndef OGG_H
#define OGG_H

#include "audio.h"

/*
	Modified code adapted from undertale_techdemo by  Xavyrr and kitling. 
*/

struct audio * ogg_create(enum channel chan);
void ogg_load(const char * name, struct audio * audio);

#endif