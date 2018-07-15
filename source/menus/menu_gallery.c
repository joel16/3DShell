#include <stdlib.h>
#include <3ds.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_gallery.h"

#include "libnsbmp.h"
#include "lodepng.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "touch.h"
#include "utils.h"

#define EXTENSION_SIZE 4

#define DIMENSION_DEFAULT             0
#define DIMENSION_NINTENDO_SCREENSHOT 1
#define DIMENSION_NINTENDO_PICTURE    2
#define DIMENSION_3DSHELL_SCREENSHOT  3
#define DIMENSION_OTHER               4

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

static void *Gallery_CreateBitmap(int width, int height, unsigned int state)
{
    (void) state;  /* unused */
    return calloc(width * height, 4);
}

static unsigned char *Gallery_GetBitmapBuf(void *bitmap)
{
    return (unsigned char *)bitmap;
}

static size_t Gallery_GetBitmapBPP(void *bitmap)
{
    (void) bitmap;  /* unused */
    return 4;
}

void  Gallery_FreeBitmap(void *bitmap)
{
    free(bitmap);
}

static void *Gallery_BitmapToBuf(const char *path, u32 *size)
{
    FILE *fd = fopen(path, "rb");
    
    if (fd == NULL)
        return NULL;
    
    u8 *buffer;
    long long_size;
    fseek(fd, 0, SEEK_END);
    long_size = ftell(fd);
    rewind(fd);

    buffer = (u8 *)malloc(long_size);
    
    if (size)
        *size = long_size;
    
    if (!buffer)
    {
        fclose(fd);
        return NULL;
    }
        
    fread(buffer, 1, long_size, fd);
    fclose(fd);
    return buffer;
}

// Thanks to LiquidFenrir
static C2D_Image *Gallery_LoadImage(const char *path)
{
    u32* outBuf = NULL;
    u32 size = 0;
    int width = 0, height = 0;
    GPU_TEXCOLOR format;

    char extension[EXTENSION_SIZE+1] = {0};
    strncpy(extension, &path[strlen(path)-EXTENSION_SIZE], EXTENSION_SIZE);

    if (!strncasecmp(extension, ".png", EXTENSION_SIZE))
    {
        unsigned char *texture;
        lodepng_decode32_file(&texture, &width, &height, path);

        for (u32 i = 0; i < width; i++)
        {
            for (u32 j = 0; j < height; j++)
            {
                u32 p = (i + j* width) * 4;

                u8 r = *(u8*)(texture + p);
                u8 g = *(u8*)(texture + p + 1);
                u8 b = *(u8*)(texture + p + 2);
                u8 a = *(u8*)(texture + p + 3);

                *(texture + p) = a;
                *(texture + p + 1) = b;
                *(texture + p + 2) = g;
                *(texture + p + 3) = r;
            }
        }

        size = (u32)(width * height * 4);
        outBuf = (u32 *)malloc(size);
        memcpy(outBuf, texture, size);
        format = GPU_RGBA8;
    }
    else if (!strncasecmp(extension, ".jpg", EXTENSION_SIZE) || !strncasecmp(extension, ".jpeg", EXTENSION_SIZE))
    {
        int channel = 0;
        stbi_uc *texture = stbi_load(path, &width, &height, &channel, STBI_rgb);

        if ((texture == NULL) || (channel != STBI_rgb))
            return NULL;

        for (u32 x = 0; x < width; x++)
        {
            for (u32 y = 0; y < height; y++)
            {

                u32 pos = (y * width + x) * channel;

                u8 c1 = texture[pos + 0];
                u8 c2 = texture[pos + 1];
                u8 c3 = texture[pos + 2];

                texture[pos + 0] = c3;
                texture[pos + 1] = c2;
                texture[pos + 2] = c1;
            }
        }

        size = (u32)(width * height * channel);
        outBuf = (u32 *)malloc(size);
        memcpy(outBuf, texture, size);
        stbi_image_free(texture);
        format = GPU_RGB8;
    }
    else if (!strncasecmp(extension, ".bmp", EXTENSION_SIZE))
    {
        u32 size;
        u8 *buf = (u8 *)Gallery_BitmapToBuf(path, &size);

        bmp_bitmap_callback_vt bitmap_callbacks = 
        {
             Gallery_CreateBitmap,
             Gallery_FreeBitmap,
             Gallery_GetBitmapBuf,
             Gallery_GetBitmapBPP
        };

        bmp_result code;
        bmp_image bmp;
        
        /* create our bmp image */
        bmp_create(&bmp, &bitmap_callbacks);

        /* analyse the BMP */
        code = bmp_analyse(&bmp, size, buf);

        if (code != BMP_OK)
        {
            bmp_finalise(&bmp);
            return NULL;
        }

        /* decode the image */
        code = bmp_decode(&bmp);

        if (code != BMP_OK)
        {
            bmp_finalise(&bmp);
            return NULL;
        }

        u8 *texture;
        texture = (u8 *)bmp.bitmap;

        for (u32 x = 0; x < bmp.width; x++)
        {
            for (u32 y = 0; y < bmp.height; y++)
            {
                u32 pos = (y * bmp.width + x) * 4;
                u8 c1 = texture[pos + 0];
                u8 c2 = texture[pos + 1];
                u8 c3 = texture[pos + 2];
                u8 c4 = texture[pos + 3];

                texture[pos + 0] = c4;
                texture[pos + 1] = c3;
                texture[pos + 2] = c2;
                texture[pos + 3] = c1;
            }
        }

        size = (bmp.width * bmp.height * 4);
        outBuf = (u32 *)malloc(size);
        memcpy(outBuf, texture, size);
        bmp_finalise(&bmp);
        format = GPU_RGBA8;
    }
    else
        return NULL;

    if (outBuf)
    {
        C2D_Image *image = malloc(sizeof(C2D_Image));
        if (image == NULL)
        {
            free(outBuf);
            return NULL;
        }

        image->tex = malloc(sizeof(C3D_Tex));

        u32 w_pow2 = Gallery_GetNextPowerOf2((u32)width);
        u32 h_pow2 = Gallery_GetNextPowerOf2((u32)height);

        Tex3DS_SubTexture *subt3x = malloc(sizeof(Tex3DS_SubTexture));
        if (subt3x == NULL)
        {
            free(image);
            free(outBuf);
            return NULL;
        }
        subt3x->width = width;
        subt3x->height = height;
        subt3x->left = 0.0f;
        subt3x->top = 1.0f;
        subt3x->right = width/(float)w_pow2;
        subt3x->bottom = 1.0 - (height/(float)h_pow2);
        image->subtex = subt3x;

        C3D_TexInit(image->tex, (u16)w_pow2, (u16)h_pow2, format);
        C3D_TexSetFilter(image->tex, GPU_LINEAR, GPU_LINEAR);

        memset(image->tex->data, 0, image->tex->size);

        u32 pixelSize = size / width / height;

        for (u32 x = 0; x < width; x++)
        {
            for (u32 y = 0; y < height; y++)
            {
                u32 dstPos = ((((y >> 3) * (w_pow2 >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * pixelSize;
                u32 srcPos = (y * width + x) * pixelSize;

                memcpy(&((u8 *)image->tex->data)[dstPos], &((u8 *) outBuf)[srcPos], pixelSize);
            }
        }

        C3D_TexFlush(image->tex);
        
        free(outBuf);
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

static bool Gallery_DrawImage(C2D_Image image, float x, float y, float start, float end)
{
    C2D_DrawParams params =
    {
        { x, y, 1.0f*image.subtex->width, 1.0f*image.subtex->height },
        { start, end },
        0.5f, 0.0f
    };

    return C2D_DrawImage(image, &params, NULL);
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

    int dimensions = 0;

	while(aptMainLoop())
	{
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_TOP, C2D_Color32(33, 39, 43, 255));
		C2D_TargetClear(RENDER_BOTTOM, C2D_Color32(33, 39, 43, 255));
		C2D_SceneBegin(RENDER_TOP);

        if ((image->subtex->width <= 400) && (image->subtex->height <= 240))
            DIMENSION_DEFAULT;
        else if ((image->subtex->width == 432) && (image->subtex->height == 528)) // Nintnedo's screenshot (both screens) dimensions.
            dimensions = DIMENSION_NINTENDO_SCREENSHOT;
        else if ((image->subtex->width == 640) && (image->subtex->height == 480)) // Nintnedo's CAM dimensions.
            dimensions = DIMENSION_NINTENDO_PICTURE;
        else if ((image->subtex->width == 400) && ((image->subtex->height == 480) || (image->subtex->height == 482)))
            dimensions = DIMENSION_3DSHELL_SCREENSHOT;
        else if ((image->subtex->width > 400) && (image->subtex->height > 240))
            dimensions = DIMENSION_OTHER;

        switch (dimensions)
        {
            case DIMENSION_DEFAULT:
                Draw_Image(*image, ((400.0f - image->subtex->width) / 2.0f), ((240.0f - image->subtex->height) / 2.0f));
                break;

            case DIMENSION_NINTENDO_SCREENSHOT:
                Gallery_DrawImage(*image, 0, 0, 16, 16);
                break;

            case DIMENSION_NINTENDO_PICTURE:
                Draw_ImageScale(*image, 40, 0, 0.5, 0.5);
                break;

            case DIMENSION_3DSHELL_SCREENSHOT:
                Gallery_DrawImage(*image, 0, 0, 0, 0);
                break;

            case DIMENSION_OTHER:
                Draw_ImageScale(*image, 0, 0, 400.0f / image->subtex->width, 240.0f / image->subtex->height);
                break;
        }

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

		C2D_SceneBegin(RENDER_BOTTOM);

        switch (dimensions)
        {
            case DIMENSION_NINTENDO_SCREENSHOT:
                Gallery_DrawImage(*image, 0, 0, 56, 272);
                break;
            
            case DIMENSION_3DSHELL_SCREENSHOT:
                Gallery_DrawImage(*image, 0, 0, 40, 240);
                break;
        }

		Draw_EndFrame();
		
		if (kDown & KEY_B)
			break;
	}

	Gallery_FreeImage(image);
	memset(album, 0, sizeof(album[0][0]) * 512 * 512);
	count = 0;
	MENU_STATE = MENU_STATE_HOME;
}
