#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    enum IMAGE_STATES {
        DIMENSION_DEFAULT = 0,
        DIMENSION_NINTENDO_SCREENSHOT = 1,
        DIMENSION_DUAL_SCREEN = 2
    };

    static std::vector<FS_DirectoryEntry> entries;
    static int selection = 0;

    static bool properties = false;
    static float okHeight = 0.f, okWidth = 0.f, scaleLevel = 0.f, width = 0.f, height = 0.f, zoomLevel = 1.f;
    static int posX = 0, posY = 0;
    static IMAGE_STATES state = DIMENSION_DEFAULT;

    constexpr u32 guiBgColour = C2D_Color32(48, 48, 48, 255);

    static bool DrawImage(C2D_Image image, float x, float y, float start, float end, float w, float h, float zoomLevel) {
        C2D_DrawParams params = {
            { x - (posX * zoomLevel), y - (posY * zoomLevel), w * zoomLevel, h * zoomLevel },
            { start, end },
            0.f, 0.f
        };

        return C2D_DrawImage(image, &params, nullptr);
    }

    static void DisplayImageProperties(int& texW, int& texH) {
        float dialogW = propertiesDialog[0].subtex->width;
        float dialogH = propertiesDialog[0].subtex->height;
        float dX = (320.f - dialogW) / 2.f;
        float dY = (240.f - dialogH) / 2.f;
        
        GUI::DrawImage(propertiesDialog[cfg.theme], dX, dY);
        GUI::DrawText(dX + 6, dY + 6, 0.42f, guiTitleColour[cfg.theme], "Properties");
        
        char filename[256];
        if (!entries.empty()) {
            Utils::UTF16ToUTF8(reinterpret_cast<u8*>(filename), reinterpret_cast<const u16*>(entries[selection].name), sizeof(filename));
        }
        else {
            std::strncpy(filename, "Unknown", sizeof(filename));
        }
        
        GUI::DrawTextf(66, 57, 0.42f, guiTextColour[cfg.theme], "Name: %.20s", filename);
        GUI::DrawTextf(66, 73, 0.42f, guiTextColour[cfg.theme], "Width: %hu px", texW);
        GUI::DrawTextf(66, 89, 0.42f, guiTextColour[cfg.theme], "Height: %hu px", texH);
        
        if (okWidth == 0.f && okHeight == 0.f) {
            GUI::GetTextDimensions(0.42f, &okWidth, &okHeight, "OK");
        }
        
        float okX = 253.f - okWidth;
        float okY = 218.f - okHeight;
        GUI::DrawRect(okX - 5, okY - 15, okWidth + 10, okHeight + 10, guiSelectorColour[cfg.theme]);
        GUI::DrawText(okX, okY - 10, 0.42f, guiTitleColour[cfg.theme], "OK");
    }

    void DisplayImageViewer(const char* path, GuiData& data) {
        FS::GetDirList(cfg.cwd, entries, FileTypeImage);
        std::string initialName = FS::GetFilename(path);
        selection = 0;
        
        for (size_t i = 0; i < entries.size(); i++) {
            char entryName[256];
            Utils::UTF16ToUTF8(reinterpret_cast<u8*>(entryName), reinterpret_cast<const u16*>(entries[i].name), sizeof(entryName));
            if (strcasecmp(entryName, initialName.c_str()) == 0) {
                selection = i;
                break;
            }
        }
        
        properties = false;
        zoomLevel = 1.f;
        posX = 0; posY = 0;
        okWidth = 0.f; okHeight = 0.f;
        bool quit = false;

        Textures::LoadImageFile(path, &data.texture);

        while (aptMainLoop() && !quit) {
            float deltaSeconds = 1.0f / 60.0f;

            int texW = data.texture.subtex->width;
            int texH = data.texture.subtex->height;

            if (texW == 432 && texH == 528) {
                state = DIMENSION_NINTENDO_SCREENSHOT;
            }
            else if (texW == 400 && (texH == 480 || texH == 482)) {
                state = DIMENSION_DUAL_SCREEN;
            }
            else {
                state = DIMENSION_DEFAULT;
                scaleLevel = (texH > 240) ? (240.f / static_cast<float>(texH)) : 1.0f;
                width = static_cast<float>(texW) * scaleLevel;
                height = static_cast<float>(texH) * scaleLevel;
            }

            GUI::Begin(guiBgColourTop[cfg.theme], guiBgColourBottom[cfg.theme]);

            switch (state) {
                case DIMENSION_DEFAULT:
                    GUI::DrawImage(data.texture, (400.f - (width * zoomLevel)) / 2.f, (240.f - (height * zoomLevel)) / 2.f, 0, 0, width, height, zoomLevel);
                    break;
                
                case DIMENSION_NINTENDO_SCREENSHOT:
                    GUI::DrawImage(data.texture, 0, 0, 16, 16, texW, texH, 1.f);
                    break;
                
                case DIMENSION_DUAL_SCREEN:
                    GUI::DrawImage(data.texture, 0, 0, 0, 0, texW, texH, 1.f);
                    break;
            }

            C2D_SceneBegin(GUI::GetRenderTarget(TARGET_BOTTOM));
            
            if (state == DIMENSION_NINTENDO_SCREENSHOT) {
                GUI::DrawImage(data.texture, 0, 0, 56, 272, texW, texH, 1.f);
            }
            else if (state == DIMENSION_DUAL_SCREEN) {
                GUI::DrawImage(data.texture, 0, 0, 40, 240, texW, texH, 1.f);
            }

            if (properties) {
                GUI::DisplayImageProperties(texW, texH);
            }

            GUI::End();

            hidScanInput();
            u32 kDown = hidKeysDown();
            u32 kHeld = hidKeysHeld();

            bool next = (kDown & KEY_R);
            bool prev = (kDown & KEY_L);

            if (state == DIMENSION_DEFAULT && !properties) {
                if ((height * zoomLevel > 240) || (width * zoomLevel > 400)) {
                    float step = 200.0f * deltaSeconds;

                    if (kHeld & KEY_CPAD_UP) {
                        posY -= step;
                    }
                    else if (kHeld & KEY_CPAD_DOWN) {
                        posY += step;
                    }
                    
                    if (kHeld & KEY_CPAD_LEFT) {
                        posX -= step;
                    }
                    else if (kHeld & KEY_CPAD_RIGHT) {
                        posX += step;
                    }
                }
                
                if ((kHeld & KEY_DUP) || (kHeld & KEY_CSTICK_UP)) {
                    zoomLevel = std::min(zoomLevel + (0.5f * deltaSeconds), 2.0f);
                }
                else if ((kHeld & KEY_DDOWN) || (kHeld & KEY_CSTICK_DOWN)) {
                    zoomLevel = std::max(zoomLevel - (0.5f * deltaSeconds), 0.5f);

                    if (zoomLevel <= 1.f) {
                        posX = posY = 0;
                    }
                }
                
                if (kDown & KEY_SELECT) { 
                    posX = posY = 0;
                    zoomLevel = 1.f;
                }
            }
            
            if (kDown & KEY_A) {
                if (properties) {
                    properties = false;
                }
            }
            else if (kDown & KEY_B) {
                if (properties) {
                    properties = false;
                }
                else {
                    quit = true;
                    break;
                }
            }
            else if (kDown & KEY_X) {
                properties = !properties;
            }
            else if (!properties && (next || prev)) {
                if (!entries.empty()) {
                    if (next) {
                        selection = (selection + 1) % entries.size();
                    }
                    else {
                        selection = (selection - 1 + entries.size()) % entries.size();
                    }
                    
                    char fullPath[1024];
                    FS::GetUTF8Path(fullPath, sizeof(fullPath), entries[selection].name);
                    Textures::LoadImageFile(fullPath, &data.texture);
                    
                    zoomLevel = 1.f;
                    posX = posY = 0;
                    state = DIMENSION_DEFAULT;
                }
            }
            
            if (state == DIMENSION_DEFAULT) {
                if (width * zoomLevel > 400.f) {
                    int maxPosX = static_cast<int>((width * zoomLevel - 400.f) / (2.f * zoomLevel));
                    posX = std::clamp(posX, -maxPosX, maxPosX);
                }
                else {
                    posX = 0;
                }

                if (height * zoomLevel > 240.f) {
                    int maxPosY = static_cast<int>((height * zoomLevel - 240.f) / (2.f * zoomLevel));
                    posY = std::clamp(posY, -maxPosY, maxPosY);
                }
                else {
                    posY = 0;
                }
            }
        }
        
        entries.clear();
        zoomLevel = 1.f;
        posX = posY = 0;
        data.state = GUI_STATE_FILEBROWSER;
    }
}
