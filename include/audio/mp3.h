#include "audio.h"

struct audio * mp3_create(enum channel chan);
void mp3_load(const char * name, struct audio * audio);