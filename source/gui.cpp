#include <3ds.h>
#include <cstdarg>
#include <cstdio>
#include <time.h>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "net.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

jmp_buf exitJmp;

namespace GUI {
    static C3D_RenderTarget *c3dRenderTarget[TARGET_MAX];
    static C2D_TextBuf guiStaticBuf, guiDynamicBuf, guiSizeBuf;

    constexpr u32 guiStatusBarColour[2] = { C2D_Color32(37, 79, 174, 255), C2D_Color32(38, 50, 56, 255) };
    constexpr u32 guiMenuBarColour[2] = { C2D_Color32(51, 103, 214, 255), C2D_Color32(55, 71, 79, 255) };
    constexpr u32 guiTouchBarColour[2] = { C2D_Color32(51, 103, 214, 255),  C2D_Color32(38, 50, 56, 255) };
    constexpr u32 guiStatusBarTextColour = C2D_Color32(255, 255, 255, 255);
    
    constexpr u32 guiItemDistance = 20, guiItemHeight = 18, guiItemStartX = 15, guiItemStartY = 84;
    constexpr float guiDepth = 0.5f;

    void Init(void) {
        romfsInit();
        gfxInitDefault();
        C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
        C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
        C2D_Prepare();

        c3dRenderTarget[TARGET_TOP] = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
        c3dRenderTarget[TARGET_BOTTOM] = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

        guiStaticBuf  = C2D_TextBufNew(4096);
        guiDynamicBuf  = C2D_TextBufNew(4096);
        guiSizeBuf = C2D_TextBufNew(4096);

        Textures::Init();
    }

    void Exit(void) {
        Textures::Exit();
        C2D_TextBufDelete(guiSizeBuf);
        C2D_TextBufDelete(guiDynamicBuf);
        C2D_TextBufDelete(guiStaticBuf);
        C2D_Fini();
        C3D_Fini();
        gfxExit();
        romfsExit();
    }
    
    void Begin(u32 topScreenColour, u32 bottomScreenColour) {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(c3dRenderTarget[TARGET_TOP], topScreenColour);
        C2D_TargetClear(c3dRenderTarget[TARGET_BOTTOM], bottomScreenColour);
        C2D_SceneBegin(c3dRenderTarget[TARGET_TOP]);
    }

    void End(void) {
        C2D_TextBufClear(guiDynamicBuf);
        C2D_TextBufClear(guiSizeBuf);
        C3D_FrameEnd(0);
    }

    C3D_RenderTarget *GetRenderTarget(u8 target) {
        return c3dRenderTarget[target];
    }

    bool DrawRect(float x, float y, float w, float h, u32 colour) {
        return C2D_DrawRectSolid(x, y, guiDepth, w, h, colour);
    }

    void GetTextDimensions(float size, float *width, float *height, const char *text) {
        C2D_Text c2dText;
        C2D_TextParse(&c2dText, guiSizeBuf, text);
        C2D_TextGetDimensions(&c2dText, size, size, width, height);
    }

    void DrawText(float x, float y, float size, u32 colour, const char *text) {
        C2D_Text c2dText;
        C2D_TextParse(&c2dText, guiDynamicBuf, text);
        C2D_TextOptimize(&c2dText);
        C2D_DrawText(&c2dText, C2D_WithColor, x, y, guiDepth, size, size, colour);
    }

    void DrawTextf(float x, float y, float size, u32 colour, const char* text, ...) {
        char buffer[128];
        va_list args;
        va_start(args, text);
        std::vsnprintf(buffer, 128, text, args);
        GUI::DrawText(x, y, size, colour, buffer);
        va_end(args);
    }

    bool DrawImage(C2D_Image image, float x, float y) {
        return C2D_DrawImageAt(image, x, y, guiDepth, nullptr, 1.f, 1.f);
    }

    bool DrawImageScale(C2D_Image image, float x, float y, float scaleX, float scaleY) {
        return C2D_DrawImageAt(image, x, y, guiDepth, nullptr, scaleX, scaleY);
    }

    static bool DrawImageBlend(C2D_Image image, float x, float y, u32 colour) {
        C2D_ImageTint tint;
        C2D_PlainImageTint(std::addressof(tint), colour, 0.5f);
        return C2D_DrawImageAt(image, x, y, guiDepth, std::addressof(tint), 1.f, 1.f);
    }

    void ResetCheckbox(GuiData& data) {
        data.checkedCopy.clear();
        data.checkedCwd.clear();
        data.checkedCount = 0;
        data.checked.assign(data.entries.size(), false);
    }

    void LoadStorageBar(GuiData& data) {
        data.totalSize = FS::GetTotalStorage(archive == sdmcArchive? SYSTEM_MEDIATYPE_SD : SYSTEM_MEDIATYPE_CTR_NAND);
        data.usedSize = FS::GetUsedStorage(archive == sdmcArchive? SYSTEM_MEDIATYPE_SD : SYSTEM_MEDIATYPE_CTR_NAND);
    }

    void ProgressBar(const std::string &title, std::string message, u64 offset, u64 size) {
        if (message.length() > 35) {
            message.resize(35);
            message.append("...");
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(c3dRenderTarget[TARGET_BOTTOM], guiBgColourTop[cfg.theme]);
        C2D_SceneBegin(c3dRenderTarget[TARGET_BOTTOM]);
        GUI::DrawRect(0, 0, 320, 20, guiTouchBarColour[cfg.theme]);

        GUI::DrawImage(dialog[cfg.theme], ((320 - (dialog[0].subtex->width)) / 2), ((240 - (dialog[0].subtex->height)) / 2));
        GUI::DrawText(((320 - (dialog[0].subtex->width)) / 2) + 6, ((240 - (dialog[0].subtex->height)) / 2) + 6 - 3, 0.42f, guiTitleColour[cfg.theme], title.c_str());

        float textWidth = 0.f;
        GUI::GetTextDimensions(0.42f, &textWidth, nullptr, message.c_str());
        GUI::DrawText(((320 - (textWidth)) / 2), ((240 - (dialog[0].subtex->height)) / 2) + 40 - 3, 0.42f, guiTextColour[cfg.theme], message.c_str());

        GUI::DrawRect(((320 - (dialog[0].subtex->width)) / 2) + 20, ((240 - (dialog[0].subtex->height)) / 2) + 65, 240, 4, guiSelectorColour[cfg.theme]);
        GUI::DrawRect(((320 - (dialog[0].subtex->width)) / 2) + 20, ((240 - (dialog[0].subtex->height)) / 2) + 65, static_cast<int>((static_cast<float>(offset) / static_cast<float>(size)) * 240.f), 
            4, guiTitleColour[cfg.theme]);
        
        GUI::End();
    }

    void DownloadProgressBar(void *args) {
        while(downloadProgress) {
            downloadSize = (downloadSize < 1.0f)? 1.0f : downloadSize;
            downloadSize = (downloadSize < downloadOffset)? downloadOffset : downloadSize;
            GUI::ProgressBar("Downloading", envIsHomebrew()? "3DShell.3dsx" : "3DShell.cia", downloadOffset, downloadSize);
        }
    }

    void DisplayStatusBar(void) {
        time_t now = time(nullptr);
        struct tm local;
        localtime_r(&now, &local);

        int hour = local.tm_hour;
        int minute = local.tm_min;
        
        // 12-hour format
        bool pm = hour >= 12;
        hour %= 12;
        if (hour == 0) {
            hour = 12;
        }

        static char timeString[30];
        std::snprintf(timeString, sizeof(timeString), "%2d:%02d %s", hour, minute, pm? "PM" : "AM");
        
        float textHeight = 0.0f;
        GUI::GetTextDimensions(0.45f, nullptr, &textHeight, timeString);
        GUI::DrawText(5, ((15 - textHeight) / 2), 0.45f, guiStatusBarTextColour, timeString);

        u8 level = 0;
        PTMU_GetBatteryLevel(&level);

#if !defined BUILD_CITRA
        u8 percent = 0;
        MCUHWC_GetBatteryLevel(&percent);
#else
        u8 percent = 100;
#endif
        
        char percentString[5];
        std::snprintf(percentString, sizeof(percentString), "%u%%", percent);

        float textWidth = 0.0f;
        GUI::GetTextDimensions(0.45f, &textWidth, nullptr, percentString);
        GUI::DrawText(395 - textWidth, ((15 - textHeight) / 2), 0.45f, guiStatusBarTextColour, percentString);

        GUI::DrawImage(batteryIcon[level], 395 - textWidth - batteryIcon[0].subtex->width - 5, 0);
        GUI::DrawImage(wifiIcon[osGetWifiStrength()], 395 - textWidth - wifiIcon[0].subtex->width - 25, 1);
    }

    static void DisplayTouchBar(GuiData& data) {
        GUI::DrawImage(data.state == GUI_STATE_FILEBROWSER? iconHomeOverlay : iconHome, 2, 0);
        GUI::DrawImage((data.state == GUI_STATE_OPTIONS) || (data.state == GUI_STATE_PROPERTIES) || (data.state == GUI_STATE_DELETE)? 
            iconOptionsOverlay : iconOptions, 25, 0);
        GUI::DrawImage((data.state == GUI_STATE_SETTINGS)? iconSettingsOverlay : iconSettings, 50, 0);
        //GUI::DrawImage(data.state == GUI_STATE_FTP? iconFTPOverlay : iconFTP[], 75, 0);
        GUI::DrawImage(archive == sdmcArchive? iconSDOverlay : iconSD, 250, 0);
        GUI::DrawImage(archive == nandArchive? iconSecureOverlay : iconSecure, 275, 0);
        GUI::DrawImage(iconSearch, 300, 0);
    }

    static void ControlTouchButtons(GuiData& data, u32& kDown) {
        if ((kDown & KEY_TOUCH) && (Touch::Rect(0, 0, 20, 20))) {
            data.state = GUI_STATE_FILEBROWSER;
        }
        else if ((kDown & KEY_TOUCH) && (Touch::Rect(20, 0, 20, 20))) {
            data.state = GUI_STATE_OPTIONS;
        }
        else if ((kDown & KEY_TOUCH) && (Touch::Rect(40, 0, 20, 20))) {
            data.state = GUI_STATE_SETTINGS;
        }
        else if ((kDown & KEY_TOUCH) && (Touch::Rect(250, 0, 20, 20))) {
            if (archive != sdmcArchive) {
                archive = sdmcArchive;
                cfg.cwd = u"/";
                data.selected = 0;
                FS::GetDirList(cfg.cwd, data.entries);
                GUI::ResetCheckbox(data);
                GUI::LoadStorageBar(data);
            }
        }
        else if ((kDown & KEY_TOUCH) && (Touch::Rect(275, 0, 20, 20))) {
            if ((archive != nandArchive) && (cfg.debug)) {
                archive = nandArchive;
                cfg.cwd = u"/";
                data.selected = 0;
                FS::GetDirList(cfg.cwd, data.entries);
                GUI::ResetCheckbox(data);
                GUI::LoadStorageBar(data);
            }
        }
        else if ((kDown & KEY_TOUCH) && (Touch::Rect(293, 0, 320, 20))) {
            // std::string path = OSK::GetText("/", "Enter file path");
            // path.append((path.back() != '/')? "/" : "");
            // if (FS::DirExists(archive, path)) {
            //     cfg.cwd = path;
            //     data.selected = 0;
            //     FS::GetDirList(cfg.cwd, data.entries);
            //     GUI::ResetCheckbox(data);
            //     GUI::LoadStorageBar(data);
            // }
        }
    }

    void MainMenu(void) {
        const float statusBarHeight = 15.0f, menuBarHeight = 25.0f, barWidth = 400.0f;

        GuiData data;
        FS::GetDirList(cfg.cwd, data.entries);
        GUI::ResetCheckbox(data);
        GUI::LoadStorageBar(data);

        u64 last = osGetTime(), current = 0;
        GUI::GetTextDimensions(0.45f, nullptr, &data.textHeight, "dummy.txt");

        while (aptMainLoop()) {
            current = osGetTime();
            u64 delta = current - last;
            last = current;

            GUI::Begin(guiBgColourTop[cfg.theme], guiBgColourBottom[cfg.theme]);
            GUI::DrawRect(0, 0, barWidth, statusBarHeight, guiStatusBarColour[cfg.theme]);
            GUI::DrawRect(0, statusBarHeight, barWidth, menuBarHeight, guiMenuBarColour[cfg.theme]);
            GUI::DisplayStatusBar();

            GUI::DisplayFileBrowser(data);

            C2D_SceneBegin(c3dRenderTarget[TARGET_BOTTOM]);
            GUI::DrawRect(0, 0, 320, 20, guiTouchBarColour[cfg.theme]);
            GUI::DisplayTouchBar(data);

            // Bottom screen view
            switch (data.state) {
                case GUI_STATE_OPTIONS:
                    GUI::DisplayFileOptions(data);
                    break;

                case GUI_STATE_DELETE:
                    GUI::DisplayDeleteOptions(data);
                    break;

                case GUI_STATE_PROPERTIES:
                    GUI::DisplayProperties(data);
                    break;

                case GUI_STATE_SETTINGS:
                    GUI::DisplaySettings(data);
                    break;
                
                default:
                    break;
            }
            
            GUI::End();

            hidScanInput();
            Touch::Update();
            u32 kDown = hidKeysDown();
            u32 kHeld = hidKeysHeld();

            switch (data.state) {
                case GUI_STATE_FILEBROWSER:
                    GUI::ControlFileBrowser(data, kDown, kHeld);
                    break;

                case GUI_STATE_OPTIONS:
                    GUI::ControlFileOptions(data, kDown);
                    break;

                case GUI_STATE_DELETE:
                    GUI::ControlDeleteOptions(data, kDown);
                    break;

                case GUI_STATE_PROPERTIES:
                    GUI::ControlProperties(data, kDown);
                    break;

                case GUI_STATE_SETTINGS:
                    GUI::ControlSettings(data, kDown);
                    break;

                default:
                    break;
            }

            GUI::ControlTouchButtons(data, kDown);

            if ((kDown & KEY_START) || (setjmp(exitJmp))) {
                break;
            }
        }
    }
}
