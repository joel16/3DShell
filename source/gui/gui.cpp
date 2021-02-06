#include <algorithm>
#include <codecvt>
#include <ctime>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "osk.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

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

    void ProgressBar(const std::string &title, const std::string &message, u64 offset, u64 size) {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(bottom_screen, cfg.dark_theme? BLACK_BG : WHITE);
        C2D_SceneBegin(bottom_screen);
        C2D::Rect(0, 0, 320, 20, cfg.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);

        C2D::Image(cfg.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));
        C2D::Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, title.c_str());

        float text_width = 0.0f;
        C2D::GetTextSize(0.42f, &text_width, nullptr, message.c_str());
        C2D::Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, message.c_str());

        C2D::Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 65, 240, 4, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        C2D::Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 65, (static_cast<double>(offset) / static_cast<double>(size)) * 240.0, 
            4, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

        C2D::Render();
    }

    static void DisplayStatusBar(void) {
        const std::time_t time = std::time(nullptr);
        const std::tm calendar_time = *std::localtime(std::addressof(time));

        std::string time_string = std::to_string((calendar_time.tm_hour % 12) == 0? 12 : (calendar_time.tm_hour % 12)) + ":" + std::to_string(calendar_time.tm_min);
        time_string.append((calendar_time.tm_hour / 12)? "PM" : "AM");
        
        float text_height = 0.0f;
        C2D::GetTextSize(0.45f, nullptr, &text_height, time_string.c_str());
        C2D::Text(5, ((15 - text_height) / 2), 0.45f, WHITE, time_string.c_str());

        u8 level = 0;
        PTMU_GetBatteryLevel(&level);

        u8 percent = 0;
        MCUHWC_GetBatteryLevel(&percent);

        std::string percent_string = std::to_string(percent) + "%";

        float percent_width = 0.0f;
        C2D::GetTextSize(0.45f, &percent_width, nullptr, percent_string.c_str());
        C2D::Text(395 - percent_width, ((15 - text_height) / 2), 0.45f, WHITE, percent_string.c_str());

        C2D::Image(battery_icons[level], 395 - percent_width - battery_icons[level].subtex->width - 5, 0);
        C2D::Image(wifi_icons[osGetWifiStrength()], 395 - percent_width - wifi_icons[osGetWifiStrength()].subtex->width - 25, 1);
    }

    static void DisplayTouchButtons(MenuItem *item) {
        C2D::Image(item->state == MENU_STATE_FILEBROWSER? icon_home_overlay : (cfg.dark_theme? icon_home_dark : icon_home), 0, -2);
        C2D::Image((item->state == MENU_STATE_OPTIONS) || (item->state == MENU_STATE_PROPERTIES) || (item->state == MENU_STATE_DELETE)? 
            icon_options_overlay : (cfg.dark_theme? icon_options_dark : icon_options), 25, 0);
        C2D::Image((item->state == MENU_STATE_SETTINGS)? icon_settings_overlay : (cfg.dark_theme? icon_settings_dark : icon_settings), 50, 0);
        //C2D::Image(item->state == MENU_STATE_FTP? icon_ftp_overlay : (cfg.dark_theme? icon_ftp_dark : icon_ftp), 75, 0);
        C2D::Image(archive == sdmc_archive? icon_sd_overlay : (cfg.dark_theme? icon_sd_dark : icon_sd), 250, 0);
        C2D::Image(archive == nand_archive? icon_secure_overlay : (cfg.dark_theme? icon_secure_dark : icon_secure), 275, 0);
        C2D::Image(icon_search, 300, 0);
    }

    static void ControlTouchButtons(MenuItem *item, u32 *kDown) {
        if ((*kDown & KEY_TOUCH) && (Touch::Rect(0, 0, 22, 20)))
            item->state = MENU_STATE_FILEBROWSER;
        else if ((*kDown & KEY_TOUCH) && (Touch::Rect(23, 0, 47, 20)))
            item->state = MENU_STATE_OPTIONS;
        else if ((*kDown & KEY_TOUCH) && (Touch::Rect(48, 0, 72, 20)))
            item->state = MENU_STATE_SETTINGS;
        else if ((*kDown & KEY_TOUCH) && (Touch::Rect(247, 0, 272, 20))) {
            if (archive != sdmc_archive) {
                archive = sdmc_archive;
                cfg.cwd = "/";
                item->selected = 0;
                FS::GetDirList(cfg.cwd, item->entries);
                GUI::ResetCheckbox(item);
                GUI::RecalcStorageSize(item);
            }
        }
        else if ((*kDown & KEY_TOUCH) && (Touch::Rect(273, 0, 292, 20))) {
            if (archive != nand_archive) {
                archive = nand_archive;
                cfg.cwd = "/";
                item->selected = 0;
                FS::GetDirList(cfg.cwd, item->entries);
                GUI::ResetCheckbox(item);
                GUI::RecalcStorageSize(item);
            }
        }
        else if ((*kDown & KEY_TOUCH) && (Touch::Rect(293, 0, 320, 20))) {
            std::string path = OSK::GetText("/", "Enter file path");
            path.append((path.back() != '/')? "/" : "");
            if (FS::DirExists(archive, path)) {
                cfg.cwd = path;
                item->selected = 0;
                FS::GetDirList(cfg.cwd, item->entries);
                GUI::ResetCheckbox(item);
                GUI::RecalcStorageSize(item);
            }
        }
    }

    Result Loop(void) {
        Result ret = 0;

        MenuItem item;
        item.state = MENU_STATE_FILEBROWSER;
        item.selected = 0;

		if (R_FAILED(ret = FS::GetDirList(cfg.cwd, item.entries)))
			return ret;
            
        GUI::ResetCheckbox(&item);
        GUI::RecalcStorageSize(&item);

        while(aptMainLoop()) {
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_TargetClear(top_screen, cfg.dark_theme? BLACK_BG : WHITE);
            C2D_TargetClear(bottom_screen, cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);
            C2D_SceneBegin(top_screen);

            C2D::Rect(0, 0, 400, 15, cfg.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
            C2D::Rect(0, 15, 400, 25, cfg.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
            GUI::DisplayStatusBar();
            
            GUI::DisplayFileBrowser(&item);

            if (item.state == MENU_STATE_IMAGEVIEWER)
                GUI::DisplayImageViewer(&item);

            C2D_SceneBegin(bottom_screen);
            C2D::Rect(0, 0, 320, 20, cfg.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);
            GUI::DisplayTouchButtons(&item);

            switch (item.state) {
                case MENU_STATE_OPTIONS:
                    GUI::DisplayFileOptions(&item);
                    break;
                
                case MENU_STATE_DELETE:
                    GUI::DisplayDeleteOptions(&item);
                    break;
                
                case MENU_STATE_SETTINGS:
                    GUI::DisplaySettings(&item);
                    break;

                default:
                    break;
            }

            C2D::Render();

            hidScanInput();
            u32 kDown = hidKeysDown();

            switch (item.state) {
                case MENU_STATE_FILEBROWSER:
                    GUI::ControlFileBrowser(&item, &kDown);
                    break;

                case MENU_STATE_OPTIONS:
                    GUI::ControlFileOptions(&item, &kDown);
                    break;

                case MENU_STATE_DELETE:
                    GUI::ControlDeleteOptions(&item, &kDown);
                    break;

                case MENU_STATE_SETTINGS:
                    GUI::ControlSettings(&item, &kDown);
                    break;

                case MENU_STATE_IMAGEVIEWER:
                    GUI::ControlImageViewer(&item, &kDown);
                    break;

                default:
                    break;
            }

            Touch::Update();
            GUI::ControlTouchButtons(&item, &kDown);

            if ((kDown & KEY_START) || (setjmp(exit_jmp)))
                break;
        }

        item.entries.clear();
        return 0;
    }
}
