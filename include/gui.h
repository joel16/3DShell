#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <string>
#include <vector>

#include "fs.h"

enum {
    TARGET_TOP = 0,
    TARGET_BOTTOM,
    TARGET_MAX
};

enum GUI_STATES {
    GUI_STATE_FILEBROWSER,
    GUI_STATE_OPTIONS,
    GUI_STATE_DELETE,
    GUI_STATE_PROPERTIES,
    GUI_STATE_SETTINGS,
    GUI_STATE_IMAGEVIEWER,
    GUI_STATE_ARCHIVEEXTRACT,
    GUI_STATE_TEXTREADER,
    GUI_STATE_UPDATE
};

typedef struct {
    GUI_STATES state = GUI_STATE_FILEBROWSER;
    int selected = 0;
    std::vector<FS_DirectoryEntry> entries;
    std::vector<u8> checked;
    std::vector<u8> checkedCopy;
    std::u16string checkedCwd;
    float textHeight = 0.0f;
    u32 checkedCount = 0;
    u64 usedSize = 0;
    u64 totalSize = 0;
    C2D_Image texture;
} GuiData;


constexpr u32 guiBgColourTop[2] = { C2D_Color32(255, 255, 255, 255), C2D_Color32(48, 48, 48, 255) };
constexpr u32 guiBgColourBottom[2] = { C2D_Color32(37, 79, 174, 255), C2D_Color32(55, 71, 79, 255) };
constexpr u32 guiSelectorColour[2] = { C2D_Color32(220, 220, 220, 255), C2D_Color32(76, 76, 76, 255) };
constexpr u32 guiTitleColour[2] = { C2D_Color32(30, 136, 229, 255), C2D_Color32(0, 150, 136, 255) };
constexpr u32 guiTextColour[2] = { C2D_Color32(32, 32, 32, 255), C2D_Color32(185, 185, 185, 255) };

namespace GUI {
    void Init(void);
    void Exit(void);
    void Begin(u32 topScreenColour, u32 bottomScreenColour);
    void End(void);
    C3D_RenderTarget *GetRenderTarget(u8 target);

    bool DrawRect(float x, float y, float w, float h, u32 colour);
    void GetTextDimensions(float size, float *width, float *height, const char *text);
    bool DrawImage(C2D_Image image, float x, float y);
    bool DrawImageScale(C2D_Image image, float x, float y, float scaleX, float scaleY);
    void DrawText(float x, float y, float size, u32 colour, const char *text);
    void DrawTextf(float x, float y, float size, u32 colour, const char* text, ...);
    
    void ResetCheckbox(GuiData& data);
    void LoadStorageBar(GuiData& data);
    void ProgressBar(const std::string &title, std::string message, u64 offset, u64 size);
    void DownloadProgressBar(void *args);
    void DisplayStatusBar(void);
    
    void MainMenu(void);
    
    void DisplayFileBrowser(GuiData& data);
    void ControlFileBrowser(GuiData& data, u32& kDown, u32& kHeld);
    
    void DisplayFileOptions(GuiData& data);
    void ControlFileOptions(GuiData& data, u32& kDown);

    void DisplayDeleteOptions(GuiData& data);
    void ControlDeleteOptions(GuiData& data, u32& kDown);
    
    void DisplayProperties(GuiData& data);
    void ControlProperties(GuiData& data, u32& kDown);

    void DisplayAudioPlayer(const char *path);
    
    void DisplaySettings(GuiData& data);
    void ControlSettings(GuiData& data, u32& kDown);
    
    void DisplayUpdateOptions(bool& status, bool& available, const std::string &tag);
    void ControlUpdateOptions(GuiData& data, u32& kDown, bool& state, bool& status, bool& available, const std::string &tag);

    void DisplayImageViewerTop(GuiData& data);
    void DisplayImageViewerBottom(GuiData& data);
    void ControlImageViewer(GuiData& data, u32& kDown, u32& kHeld, u64& delta);
}
