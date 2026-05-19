#include "config.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    enum IMAGE_STATES {
        DIMENSION_DEFAULT = 0,
        DIMENSION_NINTENDO_SCREENSHOT = 1,
        DIMENSION_DUAL_SCREEN = 2
    };

    static bool properties = false;
    static float okHeight = 0.f, okWidth = 0.f, scaleLevel = 0.f, width = 0.f, height = 0.f, zoomLevel = 1.f;
    static int posX = 0, posY = 0;
    static IMAGE_STATES state = DIMENSION_DEFAULT;

    constexpr u32 guiBgColour = C2D_Color32(48, 48, 48, 255);

    // static bool DrawImage(C2D_Image image, float x, float y, float start, float end, float w, float h, float zoomLevel) {
    //     C2D_DrawParams params = {
    //         { x - (posX * zoomLevel - posX) / 2, y - (posY * zoomLevel - posY) / 2, w * zoomLevel, h * zoomLevel },
    //         { start, end },
    //         0.f, 0.f
    //     };
        
    //     return C2D_DrawImage(image, &params, nullptr);
    // }

    static bool DrawImage(C2D_Image image, float x, float y, float start, float end, float w, float h, float zoomLevel) {
        float zoomedWidth = w * zoomLevel;
        float zoomedHeight = h * zoomLevel;
        
        float offsetX = posX * zoomLevel;
        float offsetY = posY * zoomLevel;
        
        float adjustedX = x - offsetX;
        float adjustedY = y - offsetY;
        
        C2D_DrawParams params = {
            { adjustedX, adjustedY, zoomedWidth, zoomedHeight },
            { start, end },
            0.f, 0.f
        };
        
        return C2D_DrawImage(image, &params, nullptr);
    }

    void DisplayImageViewerTop(GuiData& data) {
        C2D_TargetClear(GUI::GetRenderTarget(TARGET_TOP), guiBgColour);
        
        if ((data.texture.subtex->width == 432) && (data.texture.subtex->height == 528)) {
            state = DIMENSION_NINTENDO_SCREENSHOT;
        }
        else if ((data.texture.subtex->width == 400) && ((data.texture.subtex->height == 480) || (data.texture.subtex->height == 482))) {
            state = DIMENSION_DUAL_SCREEN;
        }
        else {
            state = DIMENSION_DEFAULT;

            if (static_cast<float>(data.texture.subtex->height) > 240.f) {
                scaleLevel = (240.f / static_cast<float>(data.texture.subtex->height));
                width = static_cast<float>(data.texture.subtex->width) * scaleLevel;
                height = static_cast<float>(data.texture.subtex->height) * scaleLevel;
            }
            else {
                width = static_cast<float>(data.texture.subtex->width);
                height = static_cast<float>(data.texture.subtex->height);
            }
        }

        switch (state) {
            case DIMENSION_DEFAULT:
                GUI::DrawImage(data.texture, ((400.f - (width * zoomLevel)) / 2), ((240.f - (height * zoomLevel)) / 2), 0, 0, width, height, zoomLevel);
                break;

            case DIMENSION_NINTENDO_SCREENSHOT:
                GUI::DrawImage(data.texture, 0, 0, 16, 16, data.texture.subtex->width, data.texture.subtex->height, 1.f);
                break;

            case DIMENSION_DUAL_SCREEN:
                GUI::DrawImage(data.texture, 0, 0, 0, 0, data.texture.subtex->width, data.texture.subtex->height, 1.f);
                break;
        }
    }

    void DisplayImageViewerBottom(GuiData& data) {
        C2D_TargetClear(GUI::GetRenderTarget(TARGET_BOTTOM), guiBgColour);

        switch (state) {
            case DIMENSION_NINTENDO_SCREENSHOT:
                GUI::DrawImage(data.texture, 0, 0, 56, 272, data.texture.subtex->width, data.texture.subtex->height, 1.f);
                break;

            case DIMENSION_DUAL_SCREEN:
                GUI::DrawImage(data.texture, 0, 0, 40, 240, data.texture.subtex->width, data.texture.subtex->height, 1.f);
                break;

            default:
                break;
        }

        if (properties) {
            GUI::DrawImage(propertiesDialog[cfg.theme], ((320 - (propertiesDialog[0].subtex->width)) / 2), ((240 - (propertiesDialog[0].subtex->height)) / 2));
            GUI::DrawText(((320 - (propertiesDialog[0].subtex->width)) / 2) + 6, ((240 - (propertiesDialog[0].subtex->height)) / 2) + 6, 0.42f, guiTitleColour[cfg.theme], "Properties");

            GUI::DrawTextf(66, 57, 0.42f, guiTextColour[cfg.theme], "Name: %.20s", data.entries[data.selected].name);
            GUI::DrawTextf(66, 73, 0.42f, guiTextColour[cfg.theme], "Width: %hu px", data.texture.subtex->width);
            GUI::DrawTextf(66, 89, 0.42f, guiTextColour[cfg.theme], "Height: %hu px", data.texture.subtex->height);

            if (okWidth == 0.f && okHeight == 0.f) {
                GUI::GetTextDimensions(0.42f, &okWidth, &okHeight, "OK");
            }

            GUI::DrawRect((253 - okWidth) - 5, (218 - okHeight) - 15, okWidth + 10, okHeight + 10, guiSelectorColour[cfg.theme]);
            GUI::DrawText(253 - okWidth, (218 - okHeight) - 10, 0.42f, guiTitleColour[cfg.theme], "OK");
        }
    }

    void ControlImageViewer(GuiData& data, u32& kDown, u32& kHeld, u64& delta) {
        float deltaSeconds = delta / 1000.0f;

        if (state == DIMENSION_DEFAULT) {
            if ((height * zoomLevel > 240) || (width * zoomLevel > 400)) {
                float velocity = 200.0f / zoomLevel;
                
                if (kHeld & KEY_CPAD_UP) {
                    posY -= ((velocity * zoomLevel) * deltaSeconds);
                }
                else if (kHeld & KEY_CPAD_DOWN) {
                    posY += ((velocity * zoomLevel) * deltaSeconds);
                }
                else if (kHeld & KEY_CPAD_LEFT) {
                    posX -= ((velocity * zoomLevel) * deltaSeconds);
                }
                else if (kHeld & KEY_CPAD_RIGHT) {
                    posX += ((velocity * zoomLevel) * deltaSeconds);
                }
            }
            
            // Zoom in
            if ((kHeld & KEY_DUP) || (kHeld & KEY_CSTICK_UP)) {
                zoomLevel += 0.5f * deltaSeconds;
                
                if (zoomLevel > 2.f) {
                    zoomLevel = 2.f;
                }
            }
            // Zoom out
            else if ((kHeld & KEY_DDOWN) || (kHeld & KEY_CSTICK_DOWN)) {
                zoomLevel -= 0.5f * deltaSeconds;
                
                if (zoomLevel < 0.5f) {
                    zoomLevel = 0.5f;
                }
                    
                if (zoomLevel <= 1.f) {
                    posX = 0;
                    posY = 0;
                }
            }
            
            if (kDown & KEY_SELECT) { // Reset zoom/pos
                posX = 0;
                posY = 0;
                zoomLevel = 1.f;
            }
        }
        
        if (kDown & KEY_A) {
            if (properties) {
                properties = false;
            }
        }
        if (kDown & KEY_B) {
            if (!properties) {
                delete[] data.texture.tex;
                delete[] data.texture.subtex;
                zoomLevel = 1.f;
                posX = 0;
                posY = 0;
                data.state = GUI_STATE_FILEBROWSER;
            }
            else {
                properties = false;
            }
        }
        else if (kDown & KEY_X) {
            if (!properties) {
                properties = true;
            }
        }
        
        // Bound zoomed image to screen
        if (state == DIMENSION_DEFAULT) {
            if (width * zoomLevel > 400.f) {
                int maxPosX = static_cast<int>((width * zoomLevel - 400.f) / (2.f * zoomLevel));
                Utils::SetMax(posX, maxPosX, maxPosX);
                Utils::SetMin(posX, -maxPosX, -maxPosX);
            }
            else {
                posX = 0;
            }

            if (height * zoomLevel > 240.f) {
                int maxPosY = static_cast<int>((height * zoomLevel - 240.f) / (2.f * zoomLevel));
                Utils::SetMax(posY, maxPosY, maxPosY);
                Utils::SetMin(posY, -maxPosY, -maxPosY);
            }
            else {
                posY = 0;
            }
        }
    }
}
