#pragma once

#include <mupdf/fitz.h>
#include <3ds.h>
#include <citro2d.h>

enum RenderReason {
    RENDER_ZOOM, 
    RENDER_NAV   
};

typedef struct {
    C2D_Image page = { nullptr, nullptr };
    bool hasTexture = false;
    int width = 0;
    int height = 0;
    int pageCount = 0;
    int pageNumber = 0;
    float rotate = 0.0f;
    float zoom = 1.0f;
} Book;

typedef struct {
    const char *path;
    int page = 0;
    float zoom = 1.0f;
    float rotate = 0.0f;
} BookEntry;

struct RenderData {
    fz_context *ctx;
    fz_display_list *list;
    fz_matrix ctm;
    fz_rect bounds;
    fz_pixmap *pix;
    RenderReason reason;
    volatile bool done = false;
};

namespace Reader {
    void Init(void);
    void Exit(void);
    void OpenDocument(const char *path, Book &book);
    void CloseDocument(Book &book);
    void RenderPage(Book &book, RenderReason reason);
    void MovePage(Book &book, float x, float y);
    void SetZoom(Book &book, float value);
}
