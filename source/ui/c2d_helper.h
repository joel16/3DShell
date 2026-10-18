#ifndef _3D_SHELL_C2D_HELPER_H
#define _3D_SHELL_C2D_HELPER_H

#include <citro2d.h>
#include <string>

extern C3D_RenderTarget *top_screen, *bottom_screen;
extern C2D_TextBuf static_buf, dynamic_buf, size_buf;

namespace C2D {
    void Render(void);
    void Text(float x, float y, float size, u32 colour, const std::string &text);
    void Textf(float x, float y, float size, u32 colour, const char *text, ...);
    void GetTextSize(float size, float *width, float *height, const std::string &text);
    bool Rect(float x, float y, float w, float h, u32 colour);
    bool Image(C2D_Image image, float x, float y);
    bool ImageScale(C2D_Image image, float x, float y, float scale_x, float scale_y);
    bool LoadImageFile(C2D_Image *texture, const std::string &path);
    bool LoadImageFileGIF(C2D_Image *texture, const std::string &path);
    bool LoadImageFilePCX(C2D_Image *texture, const std::string &path);
    bool LoadImageMemory(C2D_Image *texture, void *data, size_t size);
}

#endif
