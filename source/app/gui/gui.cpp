#include <ctime>
#include <memory>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "net.h"
#include "screen_manager.h"
#include "textures.h"
#include "utils.h"
#include "screens/file_browser_screen.h"

jmp_buf exit_jmp;

namespace GUI {
    void ResetCheckbox(MenuItem *item) {
        item->checked.clear();
        item->checked_copy.clear();
        item->checked.resize(item->entries.size());
        item->checked.assign(item->checked.size(), false);
        item->checked_cwd.clear();
        item->checked_count = 0;
    };

    void RecalcStorageSize(MenuItem *item) {
        item->total_storage = FS::GetTotalStorage(archive == sdmc_archive? SYSTEM_MEDIATYPE_SD : SYSTEM_MEDIATYPE_CTR_NAND);
        item->used_storage = FS::GetUsedStorage(archive == sdmc_archive? SYSTEM_MEDIATYPE_SD : SYSTEM_MEDIATYPE_CTR_NAND);
    }

    void ProgressBar(const std::string &title, std::string message, u64 offset, u64 size) {
        if (message.length() > 35) {
            message.resize(35);
            message.append("...");
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(bottom_screen, cfg.dark_theme? BLACK_BG : WHITE);
        C2D_SceneBegin(bottom_screen);
        C2D::Rect(0, 0, 320, 20, cfg.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);

        C2D::Image(cfg.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));
        C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, title.c_str());

        float text_width = 0.f;
        C2D::GetTextSize(0.42f, &text_width, nullptr, message.c_str());
        C2D::Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, message.c_str());

        C2D::Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 65, 240, 4, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        C2D::Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 65, static_cast<int>((static_cast<float>(offset) / static_cast<float>(size)) * 240.f), 
            4, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

        C2D::Render();
    }

    void DownloadProgressBar(void *args) {
        while(download_progress) {
            download_size = (download_size < 1.0f)? 1.0f : download_size;
            download_size = (download_size < download_offset)? download_offset : download_size;
            GUI::ProgressBar("Downloading", envIsHomebrew()? "3DShell.3dsx" : "3DShell.cia", download_offset, download_size);
        }
    }

    Result Loop(void) {
        Result ret = 0;

        auto browser = std::make_unique<FileBrowserScreen>();
        ret = browser->Init();
        if (R_FAILED(ret))
            return ret;

        if (setjmp(exit_jmp) == 0)
            UI::ScreenManager::Instance().Run(std::move(browser));

        return ret;
    }
}
