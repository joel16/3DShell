#include <mupdf/fitz.h>
#include <algorithm>
#include <cstring>

#include "config.h"
#include "gui.h"
#include "log.h"
#include "reader.h"
#include "textures.h"

namespace Reader {
    static fz_context *ctx = nullptr;
    static fz_document *doc = nullptr;
    static fz_pixmap *pix = nullptr;
    static fz_matrix ctm;
    static fz_page *page = nullptr;
    static fz_rect pageBounds = fz_empty_rect;
    static fz_point pageCenter = fz_make_point(0.f, 0.f);
    static char bookPath[256];

    // Target a continuous 320x480 vertical workspace across both screens
    static const float VIEWPORT_W = 320.0f;
    static const float VIEWPORT_H = 480.0f;

    static void CleanupCtx(void) {
        if (ctx) {
            if (pix) {
                fz_drop_pixmap(ctx, pix);
            }
            if (page) {
                fz_drop_page(ctx, page);
            }
            if (doc) {
                fz_drop_document(ctx, doc);
            }

            fz_drop_context(ctx);
            ctx = nullptr;
        }
    }

    static void CleanupTextures(Book &book) {
        if (book.page.tex) {
            C3D_TexDelete(book.page.tex);
            delete book.page.tex;
            book.page.tex = nullptr;
        }
        
        if (book.page.subtex) {
            delete book.page.subtex;
            book.page.subtex = nullptr;
        }
    }

    static void ResetPosition(const Book& book) {
        pageCenter = fz_make_point(
            ((pageBounds.x1 - pageBounds.x0) * book.zoom) / 2.f,
            ((pageBounds.y1 - pageBounds.y0) * book.zoom) / 2.f
        );
    }

    static void CreateTexture(Book &book, fz_pixmap *pixmap) {
        if (!pixmap || !pixmap->samples) {
            return;
        }
        
        Reader::CleanupTextures(book);
        book.hasTexture = false;
        bool ret = Textures::C3DTexToC2DImage(&book.page, pixmap->w, pixmap->h, pixmap->n, pixmap->samples);
        
        if (ret) {
            book.width = pixmap->w;
            book.height = pixmap->h;
            book.hasTexture = true;
        }
    }

    void Init(void) {
        ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
        if (!ctx) {
            Log::Error("%s: Cannot create mupdf context\n", __func__);
            return;
        }
        
        fz_try(ctx)
            fz_register_document_handlers(ctx);
        fz_catch(ctx) {
            Log::Error("%s: Cannot register document handlers: %s\n", __func__, fz_caught_message(ctx));
            fz_drop_context(ctx);
            ctx = nullptr;
        }
    }

    void Exit(void) {
        Reader::CleanupCtx();
        fz_drop_context(ctx);
        ctx = nullptr;
    }

    void OpenDocument(const char *path, Book &book) {
        strncpy(bookPath, path, sizeof(bookPath) - 1);
        bookPath[sizeof(bookPath) - 1] = '\0';
        
        if (!ctx) {
            Reader::Init();
        }

        fz_try(ctx)
            doc = fz_open_document(ctx, path);
        fz_catch(ctx) {
            Log::Error("%s: Cannot open document: %s\n", __func__, fz_caught_message(ctx));
            return;
        }
        
        fz_try(ctx)
            book.pageCount = fz_count_pages(ctx, doc);
        fz_catch(ctx) {
            Log::Error("%s: Cannot count number of pages: %s\n", __func__, fz_caught_message(ctx));
            fz_drop_document(ctx, doc);
            doc = nullptr;
            return;
        }

        BookEntry entry;
        Config::GetBookEntry(path, entry);
        
        book.pageNumber = entry.page;
        book.zoom = (entry.zoom > 0.0f) ? entry.zoom : 1.0f;
        
        if (book.pageNumber < 0 || book.pageNumber >= book.pageCount) {
            book.pageNumber = 0;
        }
        
        Reader::RenderPage(book, RENDER_NAV);
    }

    void CloseDocument(Book &book) {
        Config::UpdateBookEntry(bookPath, book.pageNumber, book.zoom);
        Reader::CleanupCtx();
        Reader::CleanupTextures(book);
        pageBounds = fz_empty_rect;
        pageCenter = fz_make_point(0.f, 0.f);
    }

    void RenderPage(Book &book, RenderReason reason) {
        if (pix) {
            fz_drop_pixmap(ctx, pix); pix = nullptr;
        }
        if (page) {
            fz_drop_page(ctx, page); page = nullptr;
        }

        page = fz_load_page(ctx, doc, book.pageNumber);
        fz_rect bounds = fz_bound_page(ctx, page);

        // Calculate a safe base scale to fit the 320x480 viewport
        float scaleX = VIEWPORT_W / (bounds.x1 - bounds.x0);
        float scaleY = VIEWPORT_H / (bounds.y1 - bounds.y0);
        float renderScale = std::min(scaleX, scaleY) * 1.50f;

        // Ensure the zoom loaded from config is valid
        if (book.zoom <= 0.0f) {
            book.zoom = 1.0f;
        }
        
        // Use renderScale for MuPDF so the texture never exceeds 1024x1024
        ctm = fz_scale(renderScale, renderScale);
        ctm = fz_pre_rotate(ctm, book.rotate);
        
        fz_try(ctx)
            pix = fz_new_pixmap_from_page(ctx, page, ctm, fz_device_rgb(ctx), 0);
        fz_catch(ctx) {
            Log::Error("%s: Cannot render page: %s\n", __func__, fz_caught_message(ctx));
            return;
        }

        Reader::CreateTexture(book, pix);
        
        if (reason == RENDER_NAV) {
            Reader::ResetPosition(book);
        }
    }

    void MovePage(Book &book, float x, float y) {
        float halfViewportW = VIEWPORT_W / 2.0f;
        float halfViewportH = VIEWPORT_H / 2.0f;
        float halfPageW = book.width  / 2.0f;
        float halfPageH = book.height / 2.0f;
        
        pageCenter.x += x;
        pageCenter.y += y;
        
        if (book.width <= VIEWPORT_W) {
            pageCenter.x = halfPageW;
        }
        else {
            pageCenter.x = std::clamp(pageCenter.x, halfViewportW, book.width - halfViewportW);
        }
        
        if (book.height <= VIEWPORT_H) {
            pageCenter.y = halfPageH;
        }
        else {
            pageCenter.y = std::clamp(pageCenter.y, halfViewportH, book.height - halfViewportH);
        }
    }
    
    void UpdateZoom(float oldZoom, float newZoom) {
        if (oldZoom == 0.f || oldZoom == newZoom) {
            return;
        }
        pageCenter.x = (pageCenter.x / oldZoom) * newZoom;
        pageCenter.y = (pageCenter.y / oldZoom) * newZoom;
    }

    void SetZoom(Book &book, float value) {
        book.zoom = value;
        Reader::RenderPage(book, RENDER_NAV);
    }
}
