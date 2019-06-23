#include <opus/opusfile.h>

#include "audio.h"
#include "C2D_helper.h"

static OggOpusFile *opus;
static ogg_int64_t samples_read = 0, max_samples = 0;

int OPUS_Init(const char *path) {
	int error = 0;

	if ((opus = op_open_file(path, &error)) == NULL)
		return OP_FALSE;

	if ((error = op_current_link(opus)) < 0)
		return OP_FALSE;

	max_samples = op_pcm_total(opus, -1);

	const OpusTags *tags = op_tags(opus, 0);

	if (opus_tags_query_count(tags, "title") > 0) {
		metadata.has_meta = true;
		snprintf(metadata.title, 31, "%s\n", opus_tags_query(tags, "title", 0));
	}

	if (opus_tags_query_count(tags, "album") > 0) {
		metadata.has_meta = true;
		snprintf(metadata.album, 31, "%s\n", opus_tags_query(tags, "album", 0));
	}

	if (opus_tags_query_count(tags, "artist") > 0) {
		metadata.has_meta = true;
		snprintf(metadata.artist, 31, "%s\n", opus_tags_query(tags, "artist", 0));
	}

	if (opus_tags_query_count(tags, "date") > 0) {
		metadata.has_meta = true;
		snprintf(metadata.year, 11, "%s\n", opus_tags_query(tags, "date", 0));
	}

	if (opus_tags_query_count(tags, "comment") > 0) {
		metadata.has_meta = true;
		snprintf(metadata.comment, 31, "%s\n", opus_tags_query(tags, "comment", 0));
	}

	if (opus_tags_query_count(tags, "genre") > 0) {
		metadata.has_meta = true;
		snprintf(metadata.genre, 31, "%s\n", opus_tags_query(tags, "genre", 0));
	}

	if (opus_tags_query_count(tags, "METADATA_BLOCK_PICTURE") > 0) {
		metadata.has_meta = true;

		OpusPictureTag picture_tag = { 0 };
		opus_picture_tag_init(&picture_tag);
		const char *metadata_block = opus_tags_query(tags, "METADATA_BLOCK_PICTURE", 0);

		int error = opus_picture_tag_parse(&picture_tag, metadata_block);
		if (error == 0) {
			if (picture_tag.type == 3) {
				if ((picture_tag.format == OP_PIC_FORMAT_JPEG) || (picture_tag.format == OP_PIC_FORMAT_PNG))
					Draw_LoadImageMemory(&metadata.cover_image, picture_tag.data, picture_tag.data_length);
			}
		}

		opus_picture_tag_clear(&picture_tag);
	}

	return 0;
}

u32 OPUS_GetSampleRate(void) {
	return 48000;
}

u8 OPUS_GetChannels(void) {
	return 2;
}

void OPUS_Decode(void *buf, unsigned int length, void *userdata) {
	int read = op_read_stereo(opus, (opus_int16 *)buf, (int)length * (sizeof(s16) * 2));
	if (read)
		samples_read = op_pcm_tell(opus);

	if (samples_read >= max_samples)
		playing = false;
}

u64 OPUS_GetPosition(void) {
	return samples_read;
}

u64 OPUS_GetLength(void) {
	return max_samples;
}

void OPUS_Term(void) {
	samples_read = 0;

	if (metadata.has_meta) {
		metadata.has_meta = false;

		if (metadata.cover_image.tex) {
			C3D_TexDelete(metadata.cover_image.tex);
			linearFree((Tex3DS_SubTexture *)metadata.cover_image.subtex);
			metadata.cover_image.tex = NULL;
		}
	}

	op_free(opus);
}
