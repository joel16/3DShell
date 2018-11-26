#include <stdlib.h>
#include <3ds.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_gallery.h"

#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"
#undef LOADBMP_IMPLEMENTATION
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
static int count = 0, selection = 0, dimensions = 0;
C2D_Image image;

// Thanks to LiquidFenrir
static u32 Gallery_GetNextPowerOf2(u32 v) {// from pp2d
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
static C2D_Image Gallery_LoadImage(const char *path) {
    C2D_Image img;
    u32* outBuf = NULL;
    u32 size = 0;
    int width = 0, height = 0;
    GPU_TEXCOLOR format;

    char extension[EXTENSION_SIZE + 1] = {0};
    strncpy(extension, &path[strlen(path)-EXTENSION_SIZE], EXTENSION_SIZE);

    if (!strncasecmp(extension, ".png", EXTENSION_SIZE)) {
        unsigned char *texture;
        lodepng_decode32_file(&texture, (unsigned int *)&width, (unsigned int *)&height, path);

        for (u32 i = 0; i < (u32)width; i++) {
            for (u32 j = 0; j < (u32)height; j++) {
                u32 p = (i + j* (u32)width) * 4;

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
    else if (!strncasecmp(extension, ".jpg", EXTENSION_SIZE) || !strncasecmp(extension, ".jpeg", EXTENSION_SIZE)) {
        int channel = 0;
        stbi_uc *texture = stbi_load(path, &width, &height, &channel, STBI_rgb);

        for (u32 x = 0; x < (u32)width; x++) {
            for (u32 y = 0; y < (u32)height; y++) {
                u32 pos = (y * (u32)width + x) * channel;

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
    else if (!strncasecmp(extension, ".bmp", EXTENSION_SIZE)) {
        u8* texture = NULL;
        loadbmp_decode_file(path, &texture, (unsigned int *)&width, (unsigned int *)&height, LOADBMP_RGBA);
    
        for (u32 i = 0; i < (u32)width; i++) {
            for (u32 j = 0; j < (u32)height; j++) {
                u32 p = (i + j * (u32)width) * 4;

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

    if (outBuf) {
        C3D_Tex *tex = malloc(sizeof(C3D_Tex));
        Tex3DS_SubTexture *subtex = malloc(sizeof(Tex3DS_SubTexture));

        u32 w_pow2 = Gallery_GetNextPowerOf2((u32)width);
        u32 h_pow2 = Gallery_GetNextPowerOf2((u32)height);

        subtex->width = width;
        subtex->height = height;
        subtex->left = 0.0f;
        subtex->top = 1.0f;
        subtex->right = width/(float)w_pow2;
        subtex->bottom = 1.0 - (height/(float)h_pow2);
        
        C3D_TexInit(tex, (u16)w_pow2, (u16)h_pow2, format);
        C3D_TexSetFilter(tex, GPU_LINEAR, GPU_LINEAR);

        memset(tex->data, 0, tex->size);

        u32 pixelSize = size / width / height;

        for (u32 x = 0; x < (u32)width; x++) {
            for (u32 y = 0; y < (u32)height; y++) {
                u32 dstPos = ((((y >> 3) * (w_pow2 >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * pixelSize;
                u32 srcPos = (y * width + x) * pixelSize;

                memcpy(&((u8 *)tex->data)[dstPos], &((u8 *) outBuf)[srcPos], pixelSize);
            }
        }

        C3D_TexFlush(tex);
        
        free(outBuf);
        tex->border = 0xFFFFFFFF;
        C3D_TexSetWrap(tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);

        if ((subtex->width <= 400) && (subtex->height <= 240))
            dimensions = DIMENSION_DEFAULT;
        else if ((subtex->width == 432) && (subtex->height == 528)) // Nintnedo's screenshot (both screens) dimensions.
            dimensions = DIMENSION_NINTENDO_SCREENSHOT;
        else if ((subtex->width == 640) && (subtex->height == 480)) // Nintnedo's CAM dimensions.
            dimensions = DIMENSION_NINTENDO_PICTURE;
        else if ((subtex->width == 400) && ((subtex->height == 480) || (subtex->height == 482)))
            dimensions = DIMENSION_3DSHELL_SCREENSHOT;
        else if ((subtex->width > 400) && (subtex->height > 240))
            dimensions = DIMENSION_OTHER;

        img.tex = tex;
        img.subtex = subtex;
    }

    return img;
}

// Thanks to LiquidFenrir
static void Gallery_FreeImage(C2D_Image *image) {
    dimensions = 0;
    C3D_TexDelete(image->tex);
    free(image->tex);
    free((Tex3DS_SubTexture *)image->subtex);
}

static bool Gallery_DrawImage(C2D_Image image, float x, float y, float start, float end, float scaleX, float scaleY) {
    C2D_DrawParams params = {
        { x, y, scaleX * image.subtex->width, scaleY * image.subtex->height },
        { start, end },
        0.5f, 0.0f
    };

    return C2D_DrawImage(image, &params, NULL);
}

static Result Gallery_GetImageList(void) {
    Handle dir;
    Result ret = 0;
    
    if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_ASCII, cwd)))) {
        u32 entryCount = 0;
        FS_DirectoryEntry *entries = (FS_DirectoryEntry *)calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
        
        if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries))) {
            qsort(entries, entryCount, sizeof(FS_DirectoryEntry), Utils_Alphasort);
            char name[256] = {'\0'};

            for (u32 i = 0; i < entryCount; i++) {
                Utils_U16_To_U8((u8 *)&name[0], entries[i].name, 255);

                if ((!strncasecmp(entries[i].shortExt, "png", 3)) || (!strncasecmp(entries[i].shortExt, "jpg", 3)) || (!strncasecmp(entries[i].shortExt, "bmp", 3))) {
                    strcpy(album[count], cwd);
                    strcpy(album[count] + strlen(album[count]), name);
                    count++;
                }
            }
        }
        else {
            free(entries);
            return ret;
        }
        
        free(entries);

        if (R_FAILED(ret = FSDIR_Close(dir))) // Close directory
            return ret;
    }
    else
        return ret;

    return 0;
}

static int Gallery_GetCurrentIndex(char *path) {
    for(int i = 0; i < count; ++i) {
        if (!strcmp(album[i], path))
            return i;
    }

    return 0;
}

static void Gallery_LoadTexture(char *path) {
    selection = Gallery_GetCurrentIndex(path);
    image = Gallery_LoadImage(path);
}

static void Gallery_HandleNext(bool forward) {
    if (forward)
        selection++;
    else
        selection--;

    Utils_SetMax(&selection, 0, (count - 1));
    Utils_SetMin(&selection, (count - 1), 0);

    Gallery_FreeImage(&image);
    Gallery_LoadTexture(album[selection]);
}

void Gallery_DisplayImage(char *path) {
    Gallery_GetImageList();
    Gallery_LoadTexture(path);

    while(aptMainLoop()) {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(RENDER_TOP, C2D_Color32(33, 39, 43, 255));
        C2D_TargetClear(RENDER_BOTTOM, C2D_Color32(33, 39, 43, 255));
        C2D_SceneBegin(RENDER_TOP);

        switch (dimensions) {
            case DIMENSION_DEFAULT:
                Gallery_DrawImage(image, ((400.0f - image.subtex->width) / 2.0f), ((240.0f - image.subtex->height) / 2.0f), 0, 0, 1.0f, 1.0f);
                break;

            case DIMENSION_NINTENDO_SCREENSHOT:
                Gallery_DrawImage(image, 0, 0, 16, 16, 1.0f, 1.0f);
                break;

            case DIMENSION_NINTENDO_PICTURE:
                Gallery_DrawImage(image, 40, 0, 0, 0, 0.5, 0.5);
                break;

            case DIMENSION_3DSHELL_SCREENSHOT:
                Gallery_DrawImage(image, 0, 0, 0, 0, 1.0f, 1.0f);
                break;

            case DIMENSION_OTHER:
                Gallery_DrawImage(image, 0, 0, 0, 0, 400.0f / image.subtex->width, 240.0f / image.subtex->height);
                break;
        }

        C2D_SceneBegin(RENDER_BOTTOM);

        switch (dimensions) {
            case DIMENSION_NINTENDO_SCREENSHOT:
                Gallery_DrawImage(image, 0, 0, 56, 272, 1.0f, 1.0f);
                break;
            
            case DIMENSION_3DSHELL_SCREENSHOT:
                Gallery_DrawImage(image, 0, 0, 40, 240, 1.0f, 1.0f);
                break;
        }

        Draw_EndFrame();

        hidScanInput();
        u32 kDown = hidKeysDown();

        if ((kDown & KEY_LEFT) || (kDown & KEY_L))
            Gallery_HandleNext(false);
        else if ((kDown & KEY_RIGHT) || (kDown & KEY_R))
            Gallery_HandleNext(true);
        
        if (kDown & KEY_B)
            break;
    }

    Gallery_FreeImage(&image);
    memset(album, 0, sizeof(album[0][0]) * 512 * 512);
    count = 0;
    MENU_STATE = MENU_STATE_HOME;
}
