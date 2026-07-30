#include <cstring>
#include <png.h>
#include <turbojpeg.h>

// STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_BMP
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_JPEG
#define STBI_NO_PIC
#define STBI_NO_PNG
#define STBI_ONLY_PNM
#define STBI_ONLY_PSD
#define STBI_ONLY_TGA
#include "stb_image.h"

#include "fs.h"
#include "libnsbmp.h"
#include "log.h"
#include "nsgif.h"
#include "sprites.h"
#include "textures.h"

C2D_Image fileIcon[NUM_ICONS], iconDir[2], wifiIcon[4], batteryIcon[6], batteryIconCharging[6], \
	iconCheck[2], iconUncheck[2], iconRadioOff[2], iconRadioOn[2], iconToggleOn[2], iconToggleOff, \
	dialog[2], optionsDialog[2], propertiesDialog[2], iconHome, iconHomeOverlay, \
	iconOptions, iconOptionsOverlay, iconSettings, iconSettingsOverlay, iconFTP, iconFTPOverlay, \
	iconSD, iconSDOverlay, iconSecure, iconSecureOverlay, iconSearch, iconNavDrawer, iconAction, iconBack, \
    btnPlaybackForward, btnPlaybackPause, btnPlaybackPlay, btnPlaybackRepeat, btnPlaybackRepeatOverlay, \
    btnPlaybackRewind, btnPlaybackShuffle, btnPlaybackShuffleOverlay, defaultArtworkBlur, icAlbumLg, icMusicBgBottom;

namespace Bitmap {
    static void *Create(int width, int height, [[maybe_unused]] unsigned int state) {        
        return std::calloc(width * height, 4);
    }
    
    static void *Create(int width, int height) {        
        return std::calloc(width * height, 4);
    }
    
    static unsigned char *GetBuffer(void *bitmap) {
        return static_cast<unsigned char *>(bitmap);
    }
    
    static void Destroy(void *bitmap) {
        std::free(bitmap);
    }
}

namespace Textures {
    static C2D_SpriteSheet spritesheet;

    static Result ReadFile(const char *path, u8 *&buffer, u64 &size) {
        Handle handle;
        Result ret;
        
        buffer = nullptr;
        size = 0;
        
        ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
        if (R_FAILED(ret)) {
            Log::Error("Open failed: %s (0x%x)", path, ret);
            return ret;
        }
        
        ret = FSFILE_GetSize(handle, &size);
        if (R_FAILED(ret)) {
            FSFILE_Close(handle);
            return ret;
        }
        
        buffer = new u8[size];
        if (!buffer) {
            FSFILE_Close(handle);
            return -1;
        }
        
        u32 read = 0;
        ret = FSFILE_Read(handle, &read, 0, buffer, (u32)size);
        FSFILE_Close(handle);
        
        if (R_FAILED(ret) || read != size) {
            delete[] buffer;
            buffer = nullptr;
            return -1;
        }
        
        return 0;
    }

    u32 GetNextPowerOf2(u32 v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return (v >= 64 ? v : 64);
    }

    bool C3DTexToC2DImage(C2D_Image *out, u32 width, u32 height, u32 channels, u8 *buf) {
    if (width >= 1024 || height >= 1024 || (channels != 3 && channels != 4)) {
        return false;
    }
    
    C3D_Tex *tex = new C3D_Tex();
    Tex3DS_SubTexture *subtex = new Tex3DS_SubTexture();

    subtex->width  = width;
    subtex->height = height;
    
    u32 w2 = Textures::GetNextPowerOf2(width);
    u32 h2 = Textures::GetNextPowerOf2(height);
    
    subtex->left = 0.f;
    subtex->top = 1.f;
    subtex->right = width  / static_cast<float>(w2);
    subtex->bottom = 1.f - (height / static_cast<float>(h2));
    
    // Always use GPU_RGBA8 for the destination texture
    C3D_TexInit(tex, w2, h2, GPU_RGBA8);
    C3D_TexSetFilter(tex, GPU_NEAREST, GPU_NEAREST);
    std::memset(tex->data, 0, tex->size);
    
    u8 *dst_data = static_cast<u8*>(tex->data);

    // Swizzle AND convert RGBA/RGB -> ABGR in one pass without mutating the source buffer
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u32 dst = ((((y >> 3) * (w2 >> 3) + (x >> 3)) << 6) | 
                      ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * 4;
            
            u32 src = (y * width + x) * channels;
            
            u8 r = buf[src + 0];
            u8 g = buf[src + 1];
            u8 b = buf[src + 2];
            u8 a = (channels == 4) ? buf[src + 3] : 255; // Default alpha to 255 for RGB documents

            // Write as ABGR for 3DS GPU byte order
            dst_data[dst + 0] = a;
            dst_data[dst + 1] = b;
            dst_data[dst + 2] = g;
            dst_data[dst + 3] = r;
        }
    }
    
    C3D_TexFlush(tex);
    tex->border = 0xFFFFFFFF;
    C3D_TexSetWrap(tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER); // Restore wrap fixing edge artifacts
    
    out->tex = tex;
    out->subtex = subtex;
    return true;
}

    static bool LoadImageBMP(u8 *data, u64 size, C2D_Image *texture) {
        bmp_bitmap_callback_vt callbacks = {
            .bitmap_create     = Bitmap::Create,
            .bitmap_destroy    = Bitmap::Destroy,
            .bitmap_get_buffer = Bitmap::GetBuffer,
        };
        
        bmp_result code = BMP_OK;
        bmp_image bmp;
        bmp_create(&bmp, &callbacks);
            
        code = bmp_analyse(&bmp, size, data);
        if (code != BMP_OK) {
            Log::Error("%s(bmp_analyse) failed: %d\n", __func__, code);
            bmp_finalise(&bmp);
            return false;
        }

        code = bmp_decode(&bmp);
        if (code != BMP_OK) {
            if ((code != BMP_INSUFFICIENT_DATA) && (code != BMP_DATA_ERROR)) {
                Log::Error("%s(bmp_decode) failed: %d\n", __func__, code);
                bmp_finalise(&bmp);
                return false;
            }
            
            /* skip if the decoded image would be ridiculously large */
            if ((bmp.width * bmp.height) > 200000) {
                Log::Error("%s(bmp_decode) failed: width*height is over 200000\n", __func__);
                bmp_finalise(&bmp);
                return false;
            }
        }
        
        bool ret = Textures::C3DTexToC2DImage(texture, bmp.width, bmp.height, 4, static_cast<unsigned char *>(bmp.bitmap));
        bmp_finalise(&bmp);
        return ret;
    }

    static bool LoadImageGIF(const u8 *data, u64 size, C2D_Image *texture) {
        const nsgif_bitmap_cb_vt callbacks = {
            .create     = Bitmap::Create,
            .destroy    = Bitmap::Destroy,
            .get_buffer = Bitmap::GetBuffer,
        };
    
        nsgif_t *gif;
        nsgif_error err = nsgif_create(&callbacks, NSGIF_BITMAP_FMT_ABGR8888, &gif);
        if (err != NSGIF_OK) {
            Log::Error("%s(nsgif_create) failed: %d\n", __func__, err);
            return false;
        }
    
        err = nsgif_data_scan(gif, size, data);
        if (err != NSGIF_OK) {
            Log::Error("%s(nsgif_data_scan) failed: %d\n", __func__, err);
            nsgif_destroy(gif);
            return false;
        }

        nsgif_bitmap_t *bitmap;
        const nsgif_info_t *info = nsgif_get_info(gif);
        err = nsgif_frame_decode(gif, 0, &bitmap);
        if (err != NSGIF_OK) {
            Log::Error("%s(nsgif_frame_decode) failed: %d\n", __func__, err);
            nsgif_destroy(gif);
            return false;
        }

        bool ret = Textures::C3DTexToC2DImage(texture, info->width, info->height, 4, static_cast<u8 *>(bitmap));
        nsgif_destroy(gif);
        return ret;
    }
    
    bool LoadImageJPEG(u8 *data, u64 size, C2D_Image *texture) {
        tjhandle handle = tjInitDecompress();
        int width = 0, height = 0, subsamp = 0;
        
        tjDecompressHeader2(handle, data, size, &width, &height, &subsamp);
        
        u8 *buffer = new u8[width * height * 4];
        tjDecompress2(handle, data, size, buffer, width, 0, height, TJPF_RGBA, TJFLAG_FASTDCT);
        
        bool ret = Textures::C3DTexToC2DImage(texture, width, height, 4, buffer);
        
        delete[] buffer;
        tjDestroy(handle);
        return ret;
    }
    
    bool LoadImagePNG(const u8 *data, u64 size, C2D_Image *texture) {
        png_image image;
        std::memset(&image, 0, sizeof(image));
        image.version = PNG_IMAGE_VERSION;
        
        if (!png_image_begin_read_from_memory(&image, data, size)) {
            return false;
        }
        
        image.format = PNG_FORMAT_RGBA;
        
        png_bytep buffer = new png_byte[PNG_IMAGE_SIZE(image)];
        if (!buffer) {
            png_image_free(&image);
            return false;
        }
        
        bool ret = false;
        
        if (png_image_finish_read(&image, nullptr, buffer, 0, nullptr)) {
            ret = Textures::C3DTexToC2DImage(texture, image.width, image.height, 4, buffer);
        }
        
        delete[] buffer;
        png_image_free(&image);
        return ret;
    }

    static bool LoadImageOther(const u8 *data, u64 size, C2D_Image *texture) {
        int width = 0, height = 0, channels = 0;
        unsigned char *image = stbi_load_from_memory(data, size, &width, &height, &channels, STBI_rgb_alpha);
        
        if (!image) {
            Log::Error("%s(stbi_load_from_memory) failed: %s\n", __func__, stbi_failure_reason());
            return false;
        }

        bool ret = Textures::C3DTexToC2DImage(texture, width, height, 4, image);
        stbi_image_free(image);
        return ret;
    }

    bool LoadImageFile(const char *path, C2D_Image *texture) {
        u8 *data = nullptr;
        u64 size = 0;
        
        if (R_FAILED(Textures::ReadFile(path, data, size))) {
            return false;
        }
        
        bool ret = false;
        const char *ext = FS::GetFileExt(path);
        
        if (strncasecmp(ext, "bmp", 3) == 0) {
            ret = Textures::LoadImageBMP(data, size, texture);
        }
        else if (strncasecmp(ext, "gif", 3) == 0) {
            ret = Textures::LoadImageGIF(data, size, texture);
        }
        else if ((strncasecmp(ext, "jpeg", 4) == 0) || (strncasecmp(ext, "jpg", 3) == 0)) {
            ret = Textures::LoadImageJPEG(data, size, texture);
        }
        else if (strncasecmp(ext, "png", 3) == 0) {
            ret = Textures::LoadImagePNG(data, size, texture);
        }
        else if ((strncasecmp(ext, "pgm", 3) == 0) || (strncasecmp(ext, "ppm", 3) == 0) || (strncasecmp(ext, "psd", 3) == 0) || (strncasecmp(ext, "tga", 3) == 0)) {
            ret = Textures::LoadImageOther(data, size, texture);
        }
        
        delete[] data;
        return ret;
    }
    
    void Free(C2D_Image *image) {
        if (!image) {
            return;
        }
        
        if (image->tex) {
            C3D_TexDelete(image->tex);
            delete image->tex;
            image->tex = nullptr;
        }
        
        if (image->subtex) {
            delete image->subtex;
            image->subtex = nullptr;
        }
    }
    
    void Init(void) {
        spritesheet = C2D_SpriteSheetLoad("romfs:/res/drawable/sprites.t3x");

        fileIcon[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_default_idx);
        fileIcon[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_compress_idx);
        fileIcon[2] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_audio_idx);
        fileIcon[3] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_text_idx);
        fileIcon[4] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_image_idx);
        fileIcon[5] = fileIcon[3];
        iconDir[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_folder_idx);
        iconDir[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_folder_dark_idx);
        iconCheck[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_on_normal_idx);
        iconCheck[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_on_normal_dark_idx);
        iconUncheck[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_off_normal_idx);
        iconUncheck[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_off_normal_dark_idx);
        dialog[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_dialog_idx);
        dialog[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_dialog_dark_idx);
        optionsDialog[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_options_dialog_idx);
        optionsDialog[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_options_dialog_dark_idx);
        propertiesDialog[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_properties_dialog_idx);
        propertiesDialog[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_properties_dialog_dark_idx);
        iconRadioOff[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_off_normal_idx);
        iconRadioOff[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_off_normal_dark_idx);
        iconRadioOn[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_on_normal_idx);
        iconRadioOn[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_on_normal_dark_idx);
        iconToggleOn[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_toggle_on_normal_idx);
        iconToggleOn[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_toggle_on_normal_dark_idx);
        iconToggleOff = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_toggle_off_normal_idx);
        iconHome = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_home_idx);
        iconHomeOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_home_overlay_idx);
        iconOptions = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_filesystem_idx);
        iconOptionsOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_filesystem_overlay_idx);
        iconSettings = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_settings_idx);
        iconSettingsOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_settings_overlay_idx);
        iconFTP = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_remote_idx);
        iconFTPOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_remote_overlay_idx);
        iconSD = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_sdcard_idx);
        iconSDOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_sdcard_overlay_idx);
        iconSecure = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_secure_idx);
        iconSecureOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_secure_overlay_idx);
        iconSearch = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_search_idx);
        iconNavDrawer = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_navigation_drawer_idx);
        iconAction = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_contextual_action_idx);
        iconBack = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_arrow_back_normal_idx);
        wifiIcon[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_0_idx);
        wifiIcon[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_1_idx);
        wifiIcon[2] = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_2_idx);
        wifiIcon[3] = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_3_idx);
        batteryIcon[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_20_idx);
        batteryIcon[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_30_idx);
        batteryIcon[2] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_60_idx);
        batteryIcon[3] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_80_idx);
        batteryIcon[4] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_90_idx);
        batteryIcon[5] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_full_idx);
        batteryIconCharging[0] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_20_charging_idx);
        batteryIconCharging[1] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_30_charging_idx);
        batteryIconCharging[2] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_60_charging_idx);
        batteryIconCharging[3] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_80_charging_idx);
        batteryIconCharging[4] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_90_charging_idx);
        batteryIconCharging[5] = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_full_charging_idx);
        btnPlaybackForward = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_forward_idx);
        btnPlaybackPause = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_pause_idx);
        btnPlaybackPlay = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_play_idx);
        btnPlaybackRepeat = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_repeat_idx);
        btnPlaybackRepeatOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_repeat_overlay_idx);
        btnPlaybackRewind = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_rewind_idx);
        btnPlaybackShuffle = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_shuffle_idx);
        btnPlaybackShuffleOverlay = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_shuffle_overlay_idx);
        defaultArtworkBlur = C2D_SpriteSheetGetImage(spritesheet, sprites_default_artwork_blur_idx);
        icAlbumLg = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_album_lg_idx);
        icMusicBgBottom = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_music_bg_bottom_idx);
    }

    void Exit(void) {
        C2D_SpriteSheetFree(spritesheet);
    }
}
