#include <stdlib.h>
#include <3ds.h>

#include <png.h>
#include <turbojpeg.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_gallery.h"
#include "touch.h"
#include "utils.h"

#define EXTENSION_SIZE 4

#define PNG_SIG_SIZE 8

static char album[512][512];
static int count = 0, selection = 0;
C2D_Image *image;

// Thanks to LiquidFenrir
static u32 Gallery_GetNextPowerOf2(u32 v)  // from pp2d
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v >= 64 ? v : 64;
}

// Thanks to LiquidFenrir
static C2D_Image *Gallery_LoadImage(const char *path)
{
    u32* outBuf = NULL;
    int width = 0, height = 0;

    char extension[EXTENSION_SIZE+1] = {0};
    strncpy(extension, &path[strlen(path)-EXTENSION_SIZE], EXTENSION_SIZE);

    if (!strncmp(extension, ".png", EXTENSION_SIZE))
    {
        FILE* fh = fopen(path, "rb");
        if (!fh)
            return NULL;

        u8 sig[PNG_SIG_SIZE] = {0};
        size_t size = fread(sig, sizeof(u8), PNG_SIG_SIZE, fh);
        fseek(fh, 0, SEEK_SET);

        if (size < PNG_SIG_SIZE || png_sig_cmp(sig, 0, PNG_SIG_SIZE))
        {
            fclose(fh);
            return NULL;
        }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        png_infop info = png_create_info_struct(png);

        if (setjmp(png_jmpbuf(png)))
        {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fh);
            return NULL;
        }

        png_init_io(png, fh);
        png_read_info(png, info);

        width = png_get_image_width(png, info);
        height = png_get_image_height(png, info);

        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth  = png_get_bit_depth(png, info);

        // Read any color_type into 8bit depth, ABGR format.
        // See http://www.libpng.org/pub/png/libpng-manual.txt

        if (bit_depth == 16)
            png_set_strip_16(png);

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png);

        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);

        // These color_type don't have an alpha channel then fill it with 0xff.
        if (color_type == PNG_COLOR_TYPE_RGB ||
           color_type == PNG_COLOR_TYPE_GRAY ||
           color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

        if (color_type == PNG_COLOR_TYPE_GRAY ||
           color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);

        //output ABGR
        png_set_bgr(png);
        png_set_swap_alpha(png);

        png_read_update_info(png, info);

        png_bytep* row_pointers = malloc(height*sizeof(png_bytep));
        if (row_pointers == NULL)
        {
            png_destroy_read_struct(&png, &info, NULL);
            return NULL;
        }

        for(int y = 0; y < height; y++)
            row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));

        png_read_image(png, row_pointers);

        fclose(fh);
        png_destroy_read_struct(&png, &info, NULL);

        outBuf = malloc(width*height*sizeof(u32));
        for(int j = 0; j < height; j++)
        {
            png_bytep row = row_pointers[j];
            for(int i = 0; i < width; i++)
            {
                png_bytep px = &(row[i * 4]);
                memcpy(&outBuf[j*width +i], px, sizeof(u32));
            }
            free(row_pointers[j]); // free the completed row, to avoid having to loop over the whole thing again
        }
        free(row_pointers);
    }
    else if (!strncmp(extension, ".jpg", EXTENSION_SIZE) || !strncmp(extension, ".joeg", EXTENSION_SIZE))
    {
        FILE* fh = fopen(path, "rb");
        if (fh == NULL)
            return NULL;

        fseek(fh, 0, SEEK_END);
        long int jpegSize = ftell(fh);
        fseek(fh, 0, SEEK_SET);

        unsigned char* jpegBuf = (unsigned char*)malloc(jpegSize);
        if (jpegBuf == NULL)
        {
            fclose(fh);
            return NULL;
        }

        fread(jpegBuf, jpegSize, sizeof(unsigned char), fh);
        fclose(fh);

        tjhandle handle = tjInitDecompress();
        if (handle == NULL)
        {
            free(jpegBuf);
            return NULL;
        }

        if (tjDecompressHeader(handle, jpegBuf, jpegSize, &width, &height) == -1)
        {
            free(jpegBuf);
            tjDestroy(handle);
            return NULL;
        }

        outBuf = malloc(width*height*sizeof(u32));

        if (tjDecompress2(handle, jpegBuf, jpegSize, (unsigned char*)outBuf, width, 0, height, TJPF_ABGR, TJFLAG_ACCURATEDCT) == -1)
        {
            free(outBuf);
            free(jpegBuf);
            tjDestroy(handle);
            return NULL;
        }

        free(jpegBuf);
        tjDestroy(handle);
    }
    else
        return NULL;

    if (outBuf)
    {
        C2D_Image* image = malloc(sizeof(C2D_Image));
        if (image == NULL)
        {
            free(outBuf);
            return NULL;
        }

        C3D_Tex* tex = malloc(sizeof(C3D_Tex));
        if (tex == NULL)
        {
            free(image);
            free(outBuf);
            return NULL;
        }
        image->tex = tex;

        u32 w_pow2 = Gallery_GetNextPowerOf2((u32)width);
        u32 h_pow2 = Gallery_GetNextPowerOf2((u32)height);

        Tex3DS_SubTexture* subt3x = malloc(sizeof(Tex3DS_SubTexture));
        if (subt3x == NULL)
        {
            free(tex);
            free(image);
            free(outBuf);
            return NULL;
        }
        subt3x->width = width;
        subt3x->height = height;
        subt3x->left = 0.0f;
        subt3x->top = 1.0f;
        subt3x->right = width/(float)w_pow2;
        subt3x->bottom = 1.0-(height/(float)h_pow2);
        image->subtex = subt3x;

        C3D_TexInit(image->tex, w_pow2, h_pow2, GPU_RGBA8);

        memset(image->tex->data, 0, image->tex->size);

        for(int j = 0; j < height; j++)
        {
            for(int i = 0; i < width; i++)
            {
                u32 dst = ((((j >> 3) * (w_pow2 >> 3) + (i >> 3)) << 6) + ((i & 1) | ((j & 1) << 1) | ((i & 2) << 1) | ((j & 2) << 2) | ((i & 4) << 2) | ((j & 4) << 3))) * 4;
                memcpy(((u8*)(image->tex->data)) + dst, &outBuf[j*width +i], sizeof(u32));
            }
        }
        
        free(outBuf);

        C3D_TexSetFilter(image->tex, GPU_LINEAR, GPU_LINEAR);
        image->tex->border = 0xFFFFFFFF;
        C3D_TexSetWrap(image->tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);

        return image;
    }

    return NULL;
}

// Thanks to LiquidFenrir
static void Gallery_FreeImage(C2D_Image *image)
{
    if (image)
    {
        C3D_TexDelete(image->tex);
        free(image->tex);
        free((Tex3DS_SubTexture *)image->subtex);
        free(image);
    }
}

static Result Gallery_GetImageList(void)
{
	Handle dir;
	Result ret = 0;
	
	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_ASCII, cwd))))
	{
		u32 entryCount = 0;
		FS_DirectoryEntry* entries = (FS_DirectoryEntry*) calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
		
		if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries)))
		{
			qsort(entries, entryCount, sizeof(FS_DirectoryEntry), Utils_Alphasort);
			u8 name[256] = {'\0'};

			for (u32 i = 0; i < entryCount; i++) 
			{
				Utils_U16_To_U8(&name[0], entries[i].name, 255);
				int length = strlen(name);

				if ((strncasecmp(entries[i].shortExt, "png", 3) == 0) || (strncasecmp(entries[i].shortExt, "jpg", 3) == 0) 
					|| (strncasecmp(entries[i].shortExt, "bmp", 3) == 0) || (strncasecmp(entries[i].shortExt, "gif", 3) == 0))
				{
					strcpy(album[count], cwd);
					strcpy(album[count] + strlen(album[count]), name);
					count++;
				}
			}
		}
		else
		{
			free(entries);
			return ret;
		}
		
		free(entries);

		if (R_FAILED(ret = FSDIR_Close(dir))) // Close directory
			return ret;
	}
	else
		return ret;
}

static int Gallery_GetCurrentIndex(char *path)
{
	for(int i = 0; i < count; ++i)
	{
		if (!strcmp(album[i], path))
			return i;
	}
}

static void Gallery_HandleNext(bool forward)
{
	if (forward)
		selection++;
	else
		selection--;

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	Gallery_FreeImage(image);

	Gallery_GetImageList();
	selection = Gallery_GetCurrentIndex(album[selection]);

	image = Gallery_LoadImage(album[selection]);
}

void Gallery_DisplayImage(char *path)
{
	image = Gallery_LoadImage(path);

	Gallery_GetImageList();
	selection = Gallery_GetCurrentIndex(path);

	while(aptMainLoop())
	{
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, C2D_Color32(33, 39, 43, 255));
		C2D_TargetClear(RENDER_BOTTOM, C2D_Color32(33, 39, 43, 255));
		C2D_SceneBegin(RENDER_TOP);

		Draw_Image(*image, ((400 - (image->subtex->width)) / 2), (240 - (image->subtex->height)) / 2);

		hidScanInput();
		u32 kDown = hidKeysDown();

		if ((kDown & KEY_LEFT) || (kDown & KEY_L))
		{
			wait(1);
			Gallery_HandleNext(false);
		}
		else if ((kDown & KEY_RIGHT) || (kDown & KEY_R))
		{
			wait(1);
			Gallery_HandleNext(true);
		}
		
		/*if (touchInfo.state == TouchEnded && touchInfo.tapType != TapNone)
		{
			if (tapped_inside(touchInfo, 0, 0, 120, 240))
			{
				wait(1);
				Gallery_HandleNext(false);
			}
			else if (tapped_inside(touchInfo, 1160, 0, 400, 240))
			{
				wait(1);
				Gallery_HandleNext(true);
			}
		}*/

		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_EndFrame();
		
		if (kDown & KEY_B)
			break;
	}

	Gallery_FreeImage(image);
	memset(album, 0, sizeof(album[0][0]) * 512 * 512);
	count = 0;
	MENU_DEFAULT_STATE = MENU_STATE_HOME;
}
