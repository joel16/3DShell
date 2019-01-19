#include <stdlib.h>
#include <3ds.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "fs.h"

#include "touch.h"
#include "utils.h"

enum IMAGE_STATES {
    DIMENSION_DEFAULT = 0,
    DIMENSION_NINTENDO_SCREENSHOT = 1,
    DIMENSION_NINTENDO_PICTURE = 2,
    DIMENSION_3DSHELL_SCREENSHOT = 3,
    DIMENSION_OTHER = 4
};

static char album[512][512];
static int count = 0, selection = 0, dimensions = 0, pos_x = 0, pos_y = 0;
C2D_Image image;

static void Gallery_FreeImage(C2D_Image *image) {
    C3D_TexDelete(image->tex);
    linearFree(image->subtex);
    C2D_TargetClear(RENDER_TOP, C2D_Color32(33, 39, 43, 255));
    C2D_TargetClear(RENDER_BOTTOM, C2D_Color32(33, 39, 43, 255));
}

static bool Gallery_DrawImage(C2D_Image image, float x, float y, float start, float end, float scaleX, float scaleY) {
    C2D_DrawParams params = {
        { x - (pos_x * scaleX - pos_x) / 2, y - (pos_y * scaleY - pos_y) / 2, scaleX * image.subtex->width, scaleY * image.subtex->height },
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

    char extension[5] = {0};
    strncpy(extension, &path[strlen(path) - 4], 4);

    if (!strncasecmp(extension, ".bmp", 4))
        Draw_LoadImageBMPFile(&image, path);
    else if (!strncasecmp(extension, ".jpg", 4) || !strncasecmp(extension, ".jpeg", 4))
        Draw_LoadImageJPGFile(&image, path);
    else if (!strncasecmp(extension, ".png", 4))
        Draw_LoadImagePNGFile(&image, path);
    else
        Draw_LoadImageOtherFile(&image, path);

    if ((image.subtex->width <= 400) && (image.subtex->height <= 240))
        dimensions = DIMENSION_DEFAULT;
    else if ((image.subtex->width == 432) && (image.subtex->height == 528)) // Nintnedo's screenshot (both screens) dimensions.
        dimensions = DIMENSION_NINTENDO_SCREENSHOT;
    else if ((image.subtex->width == 640) && (image.subtex->height == 480)) // Nintnedo's CAM dimensions.
        dimensions = DIMENSION_NINTENDO_PICTURE;
    else if ((image.subtex->width == 400) && ((image.subtex->height == 480) || (image.subtex->height == 482)))
        dimensions = DIMENSION_3DSHELL_SCREENSHOT;
    else if ((image.subtex->width > 400) && (image.subtex->height > 240))
        dimensions = DIMENSION_OTHER;
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

    float zoom_factor = 1.0f;

    u64 last_time = osGetTime(), current_time = 0;

    pos_x = 0;
    pos_y = 0;

    while(aptMainLoop()) {
        current_time = osGetTime();
        u64 delta_time = current_time - last_time;
        last_time = current_time;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(RENDER_TOP, C2D_Color32(33, 39, 43, 255));
        C2D_TargetClear(RENDER_BOTTOM, C2D_Color32(33, 39, 43, 255));
        C2D_SceneBegin(RENDER_TOP);

        switch (dimensions) {
            case DIMENSION_DEFAULT:
                Gallery_DrawImage(image, ((400.0f - (image.subtex->width * zoom_factor)) / 2.0f), ((240.0f - (image.subtex->height * zoom_factor)) / 2.0f), 
                    0, 0, 1.0f * zoom_factor, 1.0f * zoom_factor);
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
                Gallery_DrawImage(image, 0, 0, 0, 0, 400.0f / (image.subtex->width), 240.0f / (image.subtex->height));
                break;

            default:
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

            default:
                break;
        }

        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if ((kDown & KEY_DLEFT) || (kDown & KEY_L))
            Gallery_HandleNext(false);
        else if ((kDown & KEY_DRIGHT) || (kDown & KEY_R))
            Gallery_HandleNext(true);

        if (dimensions == DIMENSION_DEFAULT) {
            if ((image.subtex->height * zoom_factor > 320) || (image.subtex->width * zoom_factor > 400)) {
                double velocity = 2 / zoom_factor;

                if (kHeld & KEY_CPAD_UP)
                    pos_y -= ((velocity * zoom_factor) * delta_time);
                else if (kHeld & KEY_CPAD_DOWN)
                    pos_y += ((velocity * zoom_factor) * delta_time);
                else if (kHeld & KEY_CPAD_LEFT)
                    pos_x -= ((velocity * zoom_factor) * delta_time);
                else if (kHeld & KEY_CPAD_RIGHT)
                    pos_x += ((velocity * zoom_factor) * delta_time);
            }

            if ((kHeld & KEY_DUP) || (kHeld & KEY_CSTICK_UP)) {
                zoom_factor += 0.5f * (delta_time * 0.001);

                if (zoom_factor > 2.0f)
                    zoom_factor = 2.0f;
            }
            else if ((kHeld & KEY_DDOWN) || (kHeld & KEY_CSTICK_DOWN)) {
                zoom_factor -= 0.5f * (delta_time * 0.001);

                if (zoom_factor < 0.5f)
                    zoom_factor = 0.5f;

                if (zoom_factor <= 1.0f) {
                    pos_x = 0;
                    pos_y = 0;
                }
            }
        }

        Utils_SetMax(&pos_x, image.subtex->width, image.subtex->width);
        Utils_SetMin(&pos_x, -image.subtex->width, -image.subtex->width);
        Utils_SetMax(&pos_y, image.subtex->height, image.subtex->height);
        Utils_SetMin(&pos_y, -image.subtex->height, -image.subtex->height);

        Draw_EndFrame();
        
        if (kDown & KEY_B)
            break;
    }

    Gallery_FreeImage(&image);
    memset(album, 0, sizeof(album[0][0]) * 512 * 512);
    count = 0;
    MENU_STATE = MENU_STATE_HOME;
}
