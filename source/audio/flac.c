#include <FLAC/metadata.h>
#include "audio.h"
#include "C2D_helper.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

static drflac *flac;
static drflac_uint64 frames_read = 0;

int FLAC_Init(const char *path) {
	flac = drflac_open_file(path);
	if (flac == NULL)
		return -1;

	FLAC__StreamMetadata *tags;
	if (FLAC__metadata_get_tags(path, &tags)) {
		for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++)  {
			char *tag = (char *)tags->data.vorbis_comment.comments[i].entry;

			if (!strncasecmp("TITLE=", tag, 6)) {
				metadata.has_meta = true;
				snprintf(metadata.title, 31, "%s\n", tag + 6);
			}

			if (!strncasecmp("ALBUM=", tag, 6)) {
				metadata.has_meta = true;
				snprintf(metadata.album, 31, "%s\n", tag + 6);
			}

			if (!strncasecmp("ARTIST=", tag, 7)) {
				metadata.has_meta = true;
				snprintf(metadata.artist, 31, "%s\n", tag + 7);
			}

			if (!strncasecmp("DATE=", tag, 5)) {
				metadata.has_meta = true;
				snprintf(metadata.year, 31, "%d\n", atoi(tag + 5));
			}

			if (!strncasecmp("COMMENT=", tag, 8)) {
				metadata.has_meta = true;
				snprintf(metadata.comment, 31, "%s\n", tag + 8);
			}

			if (!strncasecmp("GENRE=", tag, 6)) {
				metadata.has_meta = true;
				snprintf(metadata.genre, 31, "%s\n", tag + 6);
			}
		}
	}

	if (tags)
		FLAC__metadata_object_delete(tags);

	FLAC__StreamMetadata *picture;
	if (FLAC__metadata_get_picture(path, &picture, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER, "image/jpg", NULL, 512, 512, (unsigned)(-1), (unsigned)(-1))) {
		metadata.has_meta = true;
		Draw_LoadImageMemory(&metadata.cover_image, picture->data.picture.data, picture->length);
		FLAC__metadata_object_delete(picture);
	}
	else if (FLAC__metadata_get_picture(path, &picture, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER, "image/jpeg", NULL, 512, 512, (unsigned)(-1), (unsigned)(-1))) {
		metadata.has_meta = true;
		Draw_LoadImageMemory(&metadata.cover_image, picture->data.picture.data, picture->length);
		FLAC__metadata_object_delete(picture);
	}
	else if (FLAC__metadata_get_picture(path, &picture, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER, "image/png", NULL, 512, 512, (unsigned)(-1), (unsigned)(-1))) {
		metadata.has_meta = true;
		Draw_LoadImageMemory(&metadata.cover_image, picture->data.picture.data, picture->length);
		FLAC__metadata_object_delete(picture);
	}

	return 0;
}

u32 FLAC_GetSampleRate(void) {
	return flac->sampleRate;
}

u8 FLAC_GetChannels(void) {
	return flac->channels;
}

void FLAC_Decode(void *buf, unsigned int length, void *userdata) {
	frames_read += drflac_read_s16(flac, (drflac_uint64)length, (drflac_int16 *)buf);
	
	if (frames_read >= flac->totalSampleCount)
		playing = false;
}

u64 FLAC_GetPosition(void) {
	return frames_read;
}

u64 FLAC_GetLength(void) {
	return (flac->totalSampleCount / flac->channels);
}

void FLAC_Term(void) {
	frames_read = 0;

	if (metadata.has_meta) {
		metadata.has_meta = false;

		if (metadata.cover_image.tex) {
			C3D_TexDelete(metadata.cover_image.tex);
			linearFree((Tex3DS_SubTexture *)metadata.cover_image.subtex);
			metadata.cover_image.tex = NULL;
		}
	}
	
	drflac_close(flac);
}

// Functions needed for libFLAC

int chmod(const char *pathname, mode_t mode) {
	return 0;
}

int chown(const char *path, int owner, int group) {
	return 0;
}
