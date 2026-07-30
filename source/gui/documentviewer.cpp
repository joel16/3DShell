#include <algorithm>
#include <cstring>
#include <string>

#include "config.h"
#include "gui.h"
#include "reader.h"

namespace GUI {
    static float scaleLevel = 0.f, width = 0.f, height = 0.f;
    static int posX = 0, posY = 0;
    static bool properties = false;

    constexpr u32 guiBgColour = C2D_Color32(48, 48, 48, 255);

    void DisplayDocumentViewerTop(Book& book) {
        C2D_TargetClear(GUI::GetRenderTarget(TARGET_TOP), guiBgColour);

        if (!book.hasTexture) {
            GUI::DrawText(150, 110, 0.5f, C2D_Color32(255, 255, 255, 255), "Loading Page...");
            return;
        }

        int texW = book.page.subtex->width;
        int texH = book.page.subtex->height;

        scaleLevel = (texH > 480) ? (480.f / static_cast<float>(texH)) : 1.0f;
        if (texW * scaleLevel > 320.f) {
            scaleLevel = 320.f / static_cast<float>(texW);
        }

        width = static_cast<float>(texW) * scaleLevel;
        height = static_cast<float>(texH) * scaleLevel;

        // Use book.zoom instead of zoomLevel
        float xOffset = (320.f - (width * book.zoom)) / 2.f;
        float yOffset = (480.f - (height * book.zoom)) / 2.f;

        GUI::DrawImage(book.page, xOffset + 40.f, yOffset, posX, posY, 0, 0, width, height, book.zoom);
    }

    void DisplayDocumentViewerBottom(Book& book) {
        C2D_TargetClear(GUI::GetRenderTarget(TARGET_BOTTOM), guiBgColour);

        if (book.hasTexture) {
            // Use book.zoom instead of zoomLevel
            float xOffset = (320.f - (width * book.zoom)) / 2.f;
            float yOffset = (480.f - (height * book.zoom)) / 2.f;

            GUI::DrawImage(book.page, xOffset, yOffset - 240.f, posX, posY, 0, 0, width, height, book.zoom);
        }

        char pageText[64];
        std::snprintf(pageText, sizeof(pageText), "Page: %d / %d", book.pageNumber + 1, book.pageCount);
        GUI::DrawText(10, 10, 0.5f, C2D_Color32(255,255,255,255), pageText);

        if (!book.hasTexture) {
            GUI::DrawText(10, 30, 0.5f, C2D_Color32(200,200,200,255), "Processing PDF...");
        }
    }

    void ControlDocumentViewer(Book& book, GuiData& data, u32& kDown, u32& kHeld, u64& delta) {
        float deltaSeconds = delta / 1000.0f;

        if (book.hasTexture && !properties) {
            // Boundary Checks (320x480) using book.zoom
            if ((height * book.zoom > 480.f) || (width * book.zoom > 320.f)) {
                float step = 200.0f * deltaSeconds; 
                
                if (kHeld & KEY_CPAD_UP)    posY -= step;
                if (kHeld & KEY_CPAD_DOWN)  posY += step;
                if (kHeld & KEY_CPAD_LEFT)  posX -= step;
                if (kHeld & KEY_CPAD_RIGHT) posX += step;
            }
            
            // Modify book.zoom instead of zoomLevel
            if ((kHeld & KEY_DUP) || (kHeld & KEY_CSTICK_UP)) {
                book.zoom = std::min(book.zoom + (0.5f * deltaSeconds), 4.0f);
                // Call Reader::SetZoom(book, book.zoom); here if you want real-time texture scaling
            }
            else if ((kHeld & KEY_DDOWN) || (kHeld & KEY_CSTICK_DOWN)) {
                book.zoom = std::max(book.zoom - (0.5f * deltaSeconds), 1.0f);
                if (book.zoom <= 1.f) {
                    posX = 0; posY = 0;
                }
            }
            
            // Reset to 1.0f
            if (kDown & KEY_SELECT) { 
                posX = 0; posY = 0; 
                Reader::SetZoom(book, 1.0f);
            }

            // Page Navigation
            if (kDown & KEY_R) {
                if (book.pageNumber < book.pageCount - 1) {
                    book.pageNumber++;
                    book.hasTexture = false;
                    Reader::RenderPage(book, RENDER_NAV);
                }
            }
            else if (kDown & KEY_L) {
                if (book.pageNumber > 0) {
                    book.pageNumber--;
                    book.hasTexture = false;
                    Reader::RenderPage(book, RENDER_NAV);
                }
            }
        }
        
        if (kDown & KEY_B) {
            Reader::CloseDocument(book); // This saves book.zoom to config!
            posX = 0;
            posY = 0;
            data.state = GUI_STATE_FILEBROWSER;
        } 
        
        // Screen Bounding Clamps Adapted for both Screens
        if (width * book.zoom > 320.f) {
            int maxPosX = static_cast<int>((width * book.zoom - 320.f) / (2.f * book.zoom));
            posX = std::clamp(posX, -maxPosX, maxPosX);
        }
        else {
            posX = 0;
        }

        if (height * book.zoom > 480.f) {
            int maxPosY = static_cast<int>((height * book.zoom - 480.f) / (2.f * book.zoom));
            posY = std::clamp(posY, -maxPosY, maxPosY);
        }
        else {
            posY = 0;
        }
    }
}
