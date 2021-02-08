#include <cstdarg>

#include "c2d_helper.h"

C3D_RenderTarget *top_screen, *bottom_screen;
C2D_TextBuf static_buf, dynamic_buf, size_buf;

namespace C2D {
    void Render(void) {
        C2D_TextBufClear(dynamic_buf);
        C2D_TextBufClear(size_buf);
        C3D_FrameEnd(0);
    }
    
    void Text(float x, float y, float size, u32 colour, const std::string &text) {
        C2D_Text c2d_text;
        C2D_TextParse(&c2d_text, dynamic_buf, text.c_str());
        C2D_TextOptimize(&c2d_text);
        C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.5f, size, size, colour);
    }
    
    void Textf(float x, float y, float size, u32 colour, const char* text, ...) {
        char buffer[256];
        va_list args;
        va_start(args, text);
        vsnprintf(buffer, 256, text, args);
        Text(x, y, size, colour, buffer);
        va_end(args);
    }
    
    void GetTextSize(float size, float *width, float *height, const std::string &text) {
        C2D_Text c2d_text;
        C2D_TextParse(&c2d_text, size_buf, text.c_str());
        C2D_TextGetDimensions(&c2d_text, size, size, width, height);
    }
    
    bool Rect(float x, float y, float w, float h, u32 colour) {
        return C2D_DrawRectSolid(x, y, 0.5f, w, h, colour);
    }
    
    bool Image(C2D_Image image, float x, float y) {
        return C2D_DrawImageAt(image, x, y, 0.5f, nullptr, 1.f, 1.f);
    }
    
    bool ImageScale(C2D_Image image, float x, float y, float scale_x, float scale_y) {
        return C2D_DrawImageAt(image, x, y, 0.5f, nullptr, scale_x, scale_y);
    }
}
