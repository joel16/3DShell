#include <assert.h>
#include <stdarg.h>

#include "common.h"
#include "C2D_helper.h"
#include "fs.h"
#include "utils.h"

#include "libnsbmp.h"
#include "libnsgif.h"
#include "lodepng.h"
#include "nanojpeg.h"

#define BYTES_PER_PIXEL 4
#define TRANSPARENT_COLOR 0xFFFFFFFF

void Draw_EndFrame(void) {
	C2D_TextBufClear(dynamicBuf);
	C2D_TextBufClear(sizeBuf);
	C3D_FrameEnd(0);
}

void Draw_Text(float x, float y, float size, Colour colour, const char *text) {
	C2D_Text c2d_text;
	C2D_TextParse(&c2d_text, dynamicBuf, text);
	C2D_TextOptimize(&c2d_text);
	C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.5f, size, size, colour);
}

void Draw_Textf(float x, float y, float size, Colour colour, const char* text, ...) {
	char buffer[256];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 256, text, args);
	Draw_Text(x, y, size, colour, buffer);
	va_end(args);
}

void Draw_GetTextSize(float size, float *width, float *height, const char *text) {
	C2D_Text c2d_text;
	C2D_TextParse(&c2d_text, sizeBuf, text);
	C2D_TextGetDimensions(&c2d_text, size, size, width, height);
}

float Draw_GetTextWidth(float size, const char *text) {
	float width = 0;
	Draw_GetTextSize(size, &width, NULL, text);
	return width;
}

float Draw_GetTextHeight(float size, const char *text) {
	float height = 0;
	Draw_GetTextSize(size, NULL, &height, text);
	return height;
}

bool Draw_Rect(float x, float y, float w, float h, Colour colour) {
	return C2D_DrawRectSolid(x, y, 0.5f, w, h, colour);
}

bool Draw_Image(C2D_Image image, float x, float y) {
	return C2D_DrawImageAt(image, x, y, 0.5f, NULL, 1.0f, 1.0f);
}

bool Draw_ImageScale(C2D_Image image, float x, float y, float scaleX, float scaleY) {
	return C2D_DrawImageAt(image, x, y, 0.5f, NULL, scaleX, scaleY);
}

static u64 FSFILE_FRead(void *dst, u32 size, Handle file) {
	Result ret = 0;
	u32 bytes_read = 0;
	u64 offset = 0;

	if (R_FAILED(ret = FSFILE_Read(file, &bytes_read, offset, (u32 *)dst, size)))
		return ret;

	offset += bytes_read;
	return offset;
}

static u8 *Draw_LoadExternalImageFile(const char *path, u32 *data_size) {
	Result ret = 0;
	Handle handle;
	u8 *buffer = NULL;
	u64 size = 0, n = 0;

	u16 u16_path[strlen(path) + 1];
	Utils_U8_To_U16(u16_path, (const u8 *)path, strlen(path) + 1);
	if (R_FAILED(ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, u16_path), FS_OPEN_READ, 0))) {
		FSFILE_Close(handle);
		return NULL;
	}

	if (R_FAILED(ret = FS_GetFileSize(archive, path, &size))) {
		FSFILE_Close(handle);
		return NULL;
	}

	buffer = linearAlloc(size);
	if (!buffer) {
		free(buffer);
		return NULL;
	}

	n = FSFILE_FRead(buffer, size, handle);
	if (n != size) {
		free(buffer);
		return NULL;
	}

	FSFILE_Close(handle);

	*data_size = size;
	return buffer;
}

static unsigned int Draw_GetNextPowerOf2(unsigned int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return (v >= 64 ? v : 64);
}

static void Draw_C3DTexToC2DImage(C3D_Tex *tex, Tex3DS_SubTexture *subtex, void *buf, u32 size, int width, int height, GPU_TEXCOLOR format) {
	u32 w_pow2 = Draw_GetNextPowerOf2((u32)width);
	u32 h_pow2 = Draw_GetNextPowerOf2((u32)height);

	subtex->width = (u16)width;
	subtex->height = (u16)height;
	subtex->left = 0.0f;
	subtex->top = 1.0f;
	subtex->right = (width / (float)w_pow2);
	subtex->bottom = 1.0 - (height / (float)h_pow2);

	C3D_TexInit(tex, (u16)w_pow2, (u16)h_pow2, format);
	C3D_TexSetFilter(tex, GPU_NEAREST, GPU_NEAREST);

	u32 pixel_size = (size / (u32)width / (u32)height);

	memset(tex->data, 0, tex->size);

	for (u32 x = 0; x < (u32)width; x++) {
		for (u32 y = 0; y < (u32)height; y++) {
			u32 dst_pos = ((((y >> 3) * (w_pow2 >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * pixel_size;
			u32 src_pos = (y * (u32)width + x) * pixel_size;

			memcpy(&((u8*)tex->data)[dst_pos], &((u8*)buf)[src_pos], pixel_size);
		}
	}

	C3D_TexFlush(tex);

	tex->border = TRANSPARENT_COLOR;
	C3D_TexSetWrap(tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
	linearFree(buf);
}

static void *bitmap_create(int width, int height, unsigned int state) {
	(void) state;  /* unused */
	return calloc(width * height, BYTES_PER_PIXEL);
}

static u8 *bitmap_get_buffer(void *bitmap) {
	assert(bitmap);
	return bitmap;
}

static size_t bitmap_get_bpp(void *bitmap) {
	(void) bitmap;  /* unused */
	return BYTES_PER_PIXEL;
}

static void bitmap_destroy(void *bitmap) {
	assert(bitmap);
	free(bitmap);
}

bool Draw_LoadImageBMPFile(C2D_Image *texture, const char *path) {
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
		bitmap_get_bpp
	};

	bmp_result code = BMP_OK;
	bmp_image bmp;
	u32 size = 0;

	bmp_create(&bmp, &bmp_bitmap_callbacks);
	u8 *data = Draw_LoadExternalImageFile(path, &size);

	code = bmp_analyse(&bmp, size, data);
	if (code != BMP_OK) {
		linearFree(data);
		return false;
	}

	code = bmp_decode(&bmp);

	if (code != BMP_OK) {
		if (code != BMP_INSUFFICIENT_DATA) {
			linearFree(data);
			return false;
		}
	}

	u8 *image = (u8 *)bmp.bitmap;

	for (u32 row = 0; row < (u32)bmp.width; row++) {
		for (u32 col = 0; col < (u32)bmp.height; col++) {
			u32 z = (row + col * (u32)bmp.width) * BYTES_PER_PIXEL;

			u8 r = *(u8 *)(image + z);
			u8 g = *(u8 *)(image + z + 1);
			u8 b = *(u8 *)(image + z + 2);
			u8 a = *(u8 *)(image + z + 3);

			*(image + z) = a;
			*(image + z + 1) = b;
			*(image + z + 2) = g;
			*(image + z + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(bmp.width * bmp.height * BYTES_PER_PIXEL), (u32)bmp.width, (u32)bmp.height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	bmp_finalise(&bmp);
	linearFree(data);
	return true;
}

static void *gif_bitmap_create(int width, int height) {
	return calloc(width * height, BYTES_PER_PIXEL);
}

static void gif_bitmap_set_opaque(void *bitmap, bool opaque) {
	(void) opaque;  /* unused */
	assert(bitmap);
}

static bool gif_bitmap_test_opaque(void *bitmap) {
	assert(bitmap);
	return false;
}

static u8 *gif_bitmap_get_buffer(void *bitmap) {
	assert(bitmap);
	return bitmap;
}

static void gif_bitmap_destroy(void *bitmap) {
	assert(bitmap);
	free(bitmap);
}

static void gif_bitmap_modified(void *bitmap) {
	assert(bitmap);
	return;
}

bool Draw_LoadImageGIFFile(C2D_Image *texture, const char *path) {
	gif_bitmap_callback_vt gif_bitmap_callbacks = {
		gif_bitmap_create,
		gif_bitmap_destroy,
		gif_bitmap_get_buffer,
		gif_bitmap_set_opaque,
		gif_bitmap_test_opaque,
		gif_bitmap_modified
	};

	gif_animation gif;
	u32 size = 0;
	gif_result code = GIF_OK;

	gif_create(&gif, &gif_bitmap_callbacks);
	u8 *data = Draw_LoadExternalImageFile(path, &size);

	do {
		code = gif_initialise(&gif, size, data);
		if (code != GIF_OK && code != GIF_WORKING) {
			linearFree(data);
			return false;
		}
	} while (code != GIF_OK);

	code = gif_decode_frame(&gif, 0);
	if (code != GIF_OK) {
		linearFree(data);
		return false;
	}

	u8 *image = (u8 *)gif.frame_image;

	for (u32 row = 0; row < (u32)gif.width; row++) {
		for (u32 col = 0; col < (u32)gif.height; col++) {
			u32 z = (row + col * (u32)gif.width) * BYTES_PER_PIXEL;

			u8 r = *(u8 *)(image + z);
			u8 g = *(u8 *)(image + z + 1);
			u8 b = *(u8 *)(image + z + 2);
			u8 a = *(u8 *)(image + z + 3);

			*(image + z) = a;
			*(image + z + 1) = b;
			*(image + z + 2) = g;
			*(image + z + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(gif.width * gif.height * BYTES_PER_PIXEL), (u32)gif.width, (u32)gif.height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	gif_finalise(&gif);
	linearFree(data);
	return true;
}

bool Draw_LoadImageJPGFile(C2D_Image *texture, const char *path) {
	u32 size = 0;
	u8 *data = Draw_LoadExternalImageFile(path, &size);

	njInit();
	if (njDecode(data, size)) {
		njDone();
		linearFree(data);
		return false;
	}
	
	for (u32 row = 0; row < (u32)njGetWidth(); row++) {
		for (u32 col = 0; col < (u32)njGetHeight(); col++) {
			u32 z = (col * (u32)njGetWidth() + row) * (BYTES_PER_PIXEL - 1);

			u8 r = njGetImage()[z];
			u8 g = njGetImage()[z + 1];
			u8 b = njGetImage()[z + 2];

			njGetImage()[z] = b;
			njGetImage()[z + 1] = g;
			njGetImage()[z + 2] = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, njGetImage(), (u32)(njGetWidth() * njGetHeight() * (BYTES_PER_PIXEL - 1)), (u32)njGetWidth(), (u32)njGetHeight(), GPU_RGB8);
	texture->tex = tex;
	texture->subtex = subtex;
	njDone();
	linearFree(data);
	return true;
}

bool Draw_LoadImageJPGMemory(C2D_Image *texture, void *data, size_t size) {
	njInit();
	if (njDecode(data, size))
		return false;
	
	for (u32 row = 0; row < (u32)njGetWidth(); row++) {
		for (u32 col = 0; col < (u32)njGetHeight(); col++) {
			u32 z = (col * (u32)njGetWidth() + row) * (BYTES_PER_PIXEL - 1);

			u8 r = njGetImage()[z];
			u8 g = njGetImage()[z + 1];
			u8 b = njGetImage()[z + 2];

			njGetImage()[z] = b;
			njGetImage()[z + 1] = g;
			njGetImage()[z + 2] = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, njGetImage(), (u32)(njGetWidth() * njGetHeight() * (BYTES_PER_PIXEL - 1)), (u32)njGetWidth(), (u32)njGetHeight(), GPU_RGB8);
	texture->tex = tex;
	texture->subtex = subtex;
	njDone();
	return true;
}

bool Draw_LoadImagePNGFile(C2D_Image *texture, const char *path) {
	u32 size = 0;
	u8 *data = Draw_LoadExternalImageFile(path, &size);

	u8 *image = NULL;
	unsigned width = 0, height = 0;
	unsigned error = 0;

	error = lodepng_decode32(&image, &width, &height, data, size);
	if (error) {
		linearFree(data);
		return false;
	}

	for (u32 row = 0; row < (u32)width; row++) {
		for (u32 col = 0; col < (u32)height; col++) {
			u32 z = (row + col * (u32)width) * BYTES_PER_PIXEL;

			u8 r = *(u8 *)(image + z);
			u8 g = *(u8 *)(image + z + 1);
			u8 b = *(u8 *)(image + z + 2);
			u8 a = *(u8 *)(image + z + 3);

			*(image + z) = a;
			*(image + z + 1) = b;
			*(image + z + 2) = g;
			*(image + z + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(width * height * BYTES_PER_PIXEL), (u32)width, (u32)height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	linearFree(data);
	return true;
}

bool Draw_LoadImagePNGMemory(C2D_Image *texture, void *data, size_t size) {
	u8 *image = NULL;
	unsigned width = 0, height = 0;
	unsigned error = 0;

	error = lodepng_decode32(&image, &width, &height, data, size);
	if (error)
		return false;

	for (u32 row = 0; row < (u32)width; row++) {
		for (u32 col = 0; col < (u32)height; col++) {
			u32 z = (row + col * (u32)width) * BYTES_PER_PIXEL;

			u8 r = *(u8 *)(image + z);
			u8 g = *(u8 *)(image + z + 1);
			u8 b = *(u8 *)(image + z + 2);
			u8 a = *(u8 *)(image + z + 3);

			*(image + z) = a;
			*(image + z + 1) = b;
			*(image + z + 2) = g;
			*(image + z + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(width * height * BYTES_PER_PIXEL), (u32)width, (u32)height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	return true;
}
