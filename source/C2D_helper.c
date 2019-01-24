#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "common.h"
#include "C2D_helper.h"
#include "fs.h"
#include "utils.h"

#include "libnsgif.h"
#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"
#undef LOADBMP_IMPLEMENTATION
#include "lodepng.h"
#include "nanojpeg.h"

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

	u32 pixel_size = size / width / height;

	memset(tex->data, 0, tex->size);

	for (u32 x = 0; x < (u32)width; x++) {
		for (u32 y = 0; y < (u32)height; y++) {
			u32 dst_pos = ((((y >> 3) * (w_pow2 >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * pixel_size;
			u32 src_pos = (y * width + x) * pixel_size;

			memcpy(&((u8*)tex->data)[dst_pos], &((u8*)buf)[src_pos], pixel_size);
		}
	}

	C3D_TexFlush(tex);

	tex->border = 0xFFFFFFFF;
	C3D_TexSetWrap(tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
	linearFree(buf);
}

bool Draw_LoadImageBMPFile(C2D_Image *texture, const char *path) {
	unsigned char *image = NULL;
	unsigned int width, height;

	unsigned int err = loadbmp_decode_file(path, &image, &width, &height, LOADBMP_RGBA);

	if (err)
		return false;

	for (u32 i = 0; i < (u32)width; i++) {
		for (u32 j = 0; j < (u32)height; j++) {
			u32 p = (i + j * (u32)width) * 4;

			u8 r = *(u8*)(image + p);
			u8 g = *(u8*)(image + p + 1);
			u8 b = *(u8*)(image + p + 2);
			u8 a = *(u8*)(image + p + 3);

			*(image + p) = a;
			*(image + p + 1) = b;
			*(image + p + 2) = g;
			*(image + p + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(width * height * 4), (u32)width, (u32)height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	return true;
}

static void *bitmap_create(int width, int height) {
	return calloc(width * height, 4);
}

static void bitmap_set_opaque(void *bitmap, bool opaque) {
	(void)opaque;  /* unused */
	assert(bitmap);
}

static bool bitmap_test_opaque(void *bitmap) {
	assert(bitmap);
	return false;
}

static unsigned char *bitmap_get_buffer(void *bitmap) {
	assert(bitmap);
	return bitmap;
}

static void bitmap_destroy(void *bitmap) {
	assert(bitmap);
	free(bitmap);
}

static void bitmap_modified(void *bitmap) {
	assert(bitmap);
	return;
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

static u8 *bitmap_load_file(const char *path, size_t *data_size) {
	Result ret = 0;
	Handle handle;
	unsigned char *buffer;
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

	buffer = malloc(size);
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

bool Draw_LoadImageGIFFile(C2D_Image *texture, const char *path) {
	gif_bitmap_callback_vt bitmap_callbacks = {
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
		bitmap_set_opaque,
		bitmap_test_opaque,
		bitmap_modified
	};
	gif_animation gif;
	size_t size = 0;
	gif_result code;

	gif_create(&gif, &bitmap_callbacks);
	u8 *data = bitmap_load_file(path, &size);

	do {
		code = gif_initialise(&gif, size, data);
		if (code != GIF_OK && code != GIF_WORKING)
			return false;
	} while (code != GIF_OK);

	code = gif_decode_frame(&gif, 0);
	if (code != GIF_OK)
		return false;

	u8 *image = (unsigned char *)gif.frame_image;

	for (u32 i = 0; i < (u32)gif.width; i++) {
		for (u32 j = 0; j < (u32)gif.height; j++) {
			u32 p = (i + j * (u32)gif.width) * 4;

			u8 r = *(u8*)(image + p);
			u8 g = *(u8*)(image + p + 1);
			u8 b = *(u8*)(image + p + 2);
			u8 a = *(u8*)(image + p + 3);

			*(image + p) = a;
			*(image + p + 1) = b;
			*(image + p + 2) = g;
			*(image + p + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(gif.width * gif.height * 4), (u32)gif.width, (u32)gif.height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	gif_finalise(&gif);
	free(data);
	return true;
}

bool Draw_LoadImageJPGFile(C2D_Image *texture, const char *path) {
	Result ret = 0;
	Handle handle;

	u16 u16_path[strlen(path) + 1];
	Utils_U8_To_U16(u16_path, (const u8 *)path, strlen(path) + 1);
	if (R_FAILED(ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, u16_path), FS_OPEN_READ, 0))) {
		FSFILE_Close(handle);
		return false;
	}

	u64 size = 0;
	if (R_FAILED(ret = FS_GetFileSize(archive, path, &size))) {
		FSFILE_Close(handle);
		return false;
	}

	u8 *buf = linearAlloc(size);
	if (R_FAILED(ret = FSFILE_Read(handle, NULL, 0, buf, size))) {
		linearFree(buf);
		FSFILE_Close(handle);
		return false;
	}

	njInit();
	if (njDecode(buf, size)) {
		njDone();
		linearFree(buf);
		FSFILE_Close(handle);
		return false;
	}
	
	for (u32 x = 0; x < (u32)njGetWidth(); x++) {
		for (u32 y = 0; y < (u32)njGetHeight(); y++) {
			u32 pos = (y * (u32)njGetWidth() + x) * 3;

			u8 c1 = njGetImage()[pos + 0];
			u8 c2 = njGetImage()[pos + 1];
			u8 c3 = njGetImage()[pos + 2];

			njGetImage()[pos + 0] = c3;
			njGetImage()[pos + 1] = c2;
			njGetImage()[pos + 2] = c1;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, njGetImage(), (u32)(njGetWidth() * njGetHeight() * 3), (u32)njGetWidth(), (u32)njGetHeight(), GPU_RGB8);
	texture->tex = tex;
	texture->subtex = subtex;
	linearFree(buf);
	njDone();
	FSFILE_Close(handle);
	return true;
}

bool Draw_LoadImageJPGMemory(C2D_Image *texture, void *data, size_t size) {
	njInit();
	if (njDecode(data, size))
		return false;
	
	for (u32 x = 0; x < (u32)njGetWidth(); x++) {
		for (u32 y = 0; y < (u32)njGetHeight(); y++) {
			u32 pos = (y * (u32)njGetWidth() + x) * 3;

			u8 c1 = njGetImage()[pos + 0];
			u8 c2 = njGetImage()[pos + 1];
			u8 c3 = njGetImage()[pos + 2];

			njGetImage()[pos + 0] = c3;
			njGetImage()[pos + 1] = c2;
			njGetImage()[pos + 2] = c1;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, njGetImage(), (u32)(njGetWidth() * njGetHeight() * 3), (u32)njGetWidth(), (u32)njGetHeight(), GPU_RGB8);
	texture->tex = tex;
	texture->subtex = subtex;
	njDone();
	return true;
}

bool Draw_LoadImagePNGFile(C2D_Image *texture, const char *path) {
	unsigned char *image;
	unsigned width, height;
	unsigned error;

	error = lodepng_decode32_file(&image, &width, &height, path);
	if (error)
		return false;

	for (u32 i = 0; i < (u32)width; i++) {
		for (u32 j = 0; j < (u32)height; j++) {
			u32 p = (i + j * (u32)width) * 4;

			u8 r = *(u8*)(image + p);
			u8 g = *(u8*)(image + p + 1);
			u8 b = *(u8*)(image + p + 2);
			u8 a = *(u8*)(image + p + 3);

			*(image + p) = a;
			*(image + p + 1) = b;
			*(image + p + 2) = g;
			*(image + p + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(width * height * 4), (u32)width, (u32)height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	return true;
}

bool Draw_LoadImagePNGMemory(C2D_Image *texture, void *data, size_t size) {
	unsigned char *image;
	unsigned width, height;
	unsigned error;

	error = lodepng_decode32(&image, &width, &height, data, size);
	if (error)
		return false;

	for (u32 i = 0; i < (u32)width; i++) {
		for (u32 j = 0; j < (u32)height; j++) {
			u32 p = (i + j * (u32)width) * 4;

			u8 r = *(u8*)(image + p);
			u8 g = *(u8*)(image + p + 1);
			u8 b = *(u8*)(image + p + 2);
			u8 a = *(u8*)(image + p + 3);

			*(image + p) = a;
			*(image + p + 1) = b;
			*(image + p + 2) = g;
			*(image + p + 3) = r;
		}
	}

	C3D_Tex *tex = linearAlloc(sizeof(C3D_Tex));
	Tex3DS_SubTexture *subtex = linearAlloc(sizeof(Tex3DS_SubTexture));
	Draw_C3DTexToC2DImage(tex, subtex, image, (u32)(width * height * 4), (u32)width, (u32)height, GPU_RGBA8);
	texture->tex = tex;
	texture->subtex = subtex;
	return true;
}
