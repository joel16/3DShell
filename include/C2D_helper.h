#ifndef _3D_SHELL_C2D_HELPER_H
#define _3D_SHELL_C2D_HELPER_H

#include <citro2d.h>

#define WHITE                 C2D_Color32(255, 255, 255, 255)
#define BLACK_BG              C2D_Color32(48, 48, 48, 255)
#define STATUS_BAR_LIGHT      C2D_Color32(37, 79, 174, 255)
#define STATUS_BAR_DARK       C2D_Color32(38, 50, 56, 255)
#define MENU_BAR_LIGHT        C2D_Color32(51, 103, 214, 255)
#define MENU_BAR_DARK         C2D_Color32(55, 71, 79, 255)
#define BLACK                 C2D_Color32(0, 0, 0, 255)
#define SELECTOR_COLOUR_LIGHT C2D_Color32(241, 241, 241, 255)
#define SELECTOR_COLOUR_DARK  C2D_Color32(76, 76, 76, 255)
#define TITLE_COLOUR          C2D_Color32(30, 136, 229, 255)
#define TITLE_COLOUR_DARK     C2D_Color32(0, 150, 136, 255)
#define TEXT_MIN_COLOUR_LIGHT C2D_Color32(32, 32, 32, 255)
#define TEXT_MIN_COLOUR_DARK  C2D_Color32(185, 185, 185, 255)
#define BAR_COLOUR            C2D_Color32(200, 200, 200, 255)

C3D_RenderTarget *RENDER_TOP, *RENDER_BOTTOM;
C2D_TextBuf staticBuf, dynamicBuf, sizeBuf;

typedef u32 Colour;

void Draw_EndFrame(void);
void Draw_Text(float x, float y, float size, Colour colour, const char *text);
void Draw_Textf(float x, float y, float size, Colour colour, const char* text, ...);
void Draw_GetTextSize(float size, float *width, float *height, const char *text);
float Draw_GetTextWidth(float size, const char *text);
float Draw_GetTextHeight(float size, const char *text);
bool Draw_Rect(float x, float y, float w, float h, Colour colour);
bool Draw_Image(C2D_Image image, float x, float y);
bool Draw_ImageScale(C2D_Image image, float x, float y, float scaleX, float scaleY);
bool Draw_LoadImageFile(C2D_Image *texture, const char *path);
bool Draw_LoadImageFileGIF(C2D_Image *texture, const char *path);
bool Draw_LoadImageFilePCX(C2D_Image *texture, const char *path);
bool Draw_LoadImageMemory(C2D_Image *texture, void *data, size_t size);

#endif
