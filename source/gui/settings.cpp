#include "config.h"
#include "fs.h"
#include "gui.h"
#include "net.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {
    enum SETTINGS_STATE {
        GENERAL_SETTINGS,
        SORT_SETTINGS,
        UPDATE_SETTINGS
    };
    
    static SETTINGS_STATE state = GENERAL_SETTINGS;
    static int selection = 0;
    constexpr int guiSelDist = 40;
    constexpr u32 guiTextColour[2] = { C2D_Color32(0, 0, 0, 255), C2D_Color32(255, 255, 255, 255) };
    constexpr u32 guiTitleColour = C2D_Color32(255, 255, 255, 255);
    static std::string tagName = std::string();
    static bool networkStatus = false, updateAvailable = false, updatePopup = false;

    static void DisplaySortSettings(void) {
        GUI::DrawText(35, 30, 0.44f, guiTitleColour, "Sorting Options");
        
        GUI::DrawText(10, 58, 0.44f, guiTextColour[cfg.theme], "Alphabetical");
        GUI::DrawText(10, 74, 0.42f, guiTextColour[cfg.theme], "Sort alphabetically in ascending order.");
        GUI::DrawText(10, 98, 0.44f, guiTextColour[cfg.theme], "Alphabetical");
        GUI::DrawText(10, 114, 0.42f, guiTextColour[cfg.theme], "Sort alphabetically in descending order.");
        GUI::DrawText(10, 138, 0.44f, guiTextColour[cfg.theme], "Size");
        GUI::DrawText(10, 154, 0.42f, guiTextColour[cfg.theme], "Sort by size (largest first).");
        GUI::DrawText(10, 178, 0.44f, guiTextColour[cfg.theme], "Size");
        GUI::DrawText(10, 194, 0.42f, guiTextColour[cfg.theme], "Sort by size (smallest first).");

        GUI::DrawImage(cfg.sort == 0? iconRadioOn[cfg.theme] : iconRadioOff[cfg.theme], 270, 60);
        GUI::DrawImage(cfg.sort == 1? iconRadioOn[cfg.theme] : iconRadioOff[cfg.theme], 270, 100);
        GUI::DrawImage(cfg.sort == 2? iconRadioOn[cfg.theme] : iconRadioOff[cfg.theme], 270, 140);
        GUI::DrawImage(cfg.sort == 3? iconRadioOn[cfg.theme] : iconRadioOff[cfg.theme], 270, 180);
    }

    static void ControlSortSettings(GuiData& data, u32 kDown) {
        if (kDown & KEY_DUP) {
            selection--;
        }
        else if (kDown & KEY_DDOWN) {
            selection++;
        }
        else if (kDown & KEY_A) {
            cfg.sort = selection;
            Config::Save(cfg);
            FS::GetDirList(cfg.cwd, data.entries);
        }
        else if (kDown & KEY_B) {
            selection = 0;
            state = GENERAL_SETTINGS;
        }
        
        if (Touch::Rect(0, 55, 320, 94)) {
            selection = 0;
            
            if (kDown & KEY_TOUCH) {
                cfg.sort = selection;
                Config::Save(cfg);
                FS::GetDirList(cfg.cwd, data.entries);
            }
        }
        else if (Touch::Rect(0, 95, 320, 134)) {
            selection = 1;
            
            if (kDown & KEY_TOUCH) {
                cfg.sort = selection;
                Config::Save(cfg);
                FS::GetDirList(cfg.cwd, data.entries);
            }
        }
        else if (Touch::Rect(0, 135, 320, 174)) {
            selection = 2;
            
            if (kDown & KEY_TOUCH) {
                cfg.sort = selection;
                Config::Save(cfg);
                FS::GetDirList(cfg.cwd, data.entries);
            }
        }
        else if (Touch::Rect(0, 175, 320, 215)) {
            selection = 3;
            
            if (kDown & KEY_TOUCH) {
                cfg.sort = selection;
                Config::Save(cfg);
                FS::GetDirList(cfg.cwd, data.entries);
            }
        }
        else if (Touch::Rect(5, 25, 30, 50)) {
            if (kDown & KEY_TOUCH) {
                selection = 0;
                state = GENERAL_SETTINGS;
            }
        }
        
        Utils::Wrap(selection, 0, 3);
    }

    static void DisplayUpdateSettings(void) {
        GUI::DrawText(35, 30, 0.44f, guiTitleColour, "Updates");

        GUI::DrawText(10, 58, 0.44f, guiTextColour[cfg.theme], "Check for updates");
        GUI::DrawText(10, 74, 0.42f, guiTextColour[cfg.theme], "Downloads and installs the latest version.");
        GUI::DrawText(10, 98, 0.44f, guiTextColour[cfg.theme], "About");
        GUI::DrawTextf(10, 114, 0.42f, guiTextColour[cfg.theme], "3DShell v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
        GUI::DrawText(10, 138, 0.44f, guiTextColour[cfg.theme], "Author: Joel16");
        GUI::DrawText(10, 154, 0.42f, guiTextColour[cfg.theme], "Assets: Preetisketch/CyanogenMod/LineageOS");

        if (updatePopup) {
            GUI::DisplayUpdateOptions(networkStatus, updateAvailable, tagName);
        }
    }

    static void ControlUpdateSettings(GuiData& data, u32 kDown) {
        if (updatePopup) {
            GUI::ControlUpdateOptions(data, kDown, updatePopup, networkStatus, updateAvailable, tagName);
        }
        else {
            if (kDown & KEY_DUP) {
                selection--;
            }
            else if (kDown & KEY_DDOWN) {
                selection++;
            }
            else if (kDown & KEY_A) {
                if (selection == 0) {
                    Net::Init();
                    networkStatus =  Net::GetNetworkStatus();
                    tagName = Net::GetLatestReleaseJSON();
                    updateAvailable = Net::GetAvailableUpdate(tagName);
                    updatePopup = true;
                    Net::Exit();
                }
            }
            else if (kDown & KEY_B) {
                selection = 0;
                state = GENERAL_SETTINGS;
            }
            
            Utils::Wrap(selection, 0, 2);
        }

        if (Touch::Rect(0, 55, 320, 94)) {
            selection = 0;
            
            if (kDown & KEY_TOUCH) {
                Net::Init();
                networkStatus =  Net::GetNetworkStatus();
                tagName = Net::GetLatestReleaseJSON();
                updateAvailable = Net::GetAvailableUpdate(tagName);
                updatePopup = true;
                Net::Exit();
            }
        }
        else if (Touch::Rect(5, 25, 30, 50)) {
            if (kDown & KEY_TOUCH) {
                selection = 0;
                state = GENERAL_SETTINGS;
            }
        }
    }

    static void DisplayGeneralSettings(void) {
        GUI::DrawText(10, 30, 0.44f, guiTitleColour, "Settings");

        GUI::DrawText(10, 58, 0.44f, guiTextColour[cfg.theme], "Sort by");
        GUI::DrawText(10, 74, 0.42f, guiTextColour[cfg.theme], "Select between various sorting options.");
        GUI::DrawText(10, 98, 0.44f, guiTextColour[cfg.theme], "Dark theme");
        GUI::DrawText(10, 114, 0.42f, guiTextColour[cfg.theme], "Enables dark theme mode.");
        GUI::DrawText(10, 138, 0.44f, guiTextColour[cfg.theme], "Developer options");
        GUI::DrawText(10, 154, 0.42f, guiTextColour[cfg.theme], "Enable logging and fs access to NAND.");
        GUI::DrawText(10, 178, 0.44f, guiTextColour[cfg.theme], "Check for update");
        GUI::DrawText(10, 194, 0.42f, guiTextColour[cfg.theme], "Downloads and installs the latest version.");
        
        if (cfg.theme) {
            GUI::DrawImage(iconToggleOn[cfg.theme], 270, 97);
        }
        else {
            GUI::DrawImage(iconToggleOff, 270, 97);
        }

        GUI::DrawImage(cfg.debug? iconToggleOn[cfg.theme] : iconToggleOff, 270, 137);
    }

    void ControlGeneralSettings(GuiData& data, u32& kDown) {
        if (kDown & KEY_DUP) {
            selection--;
        }
        else if (kDown & KEY_DDOWN) {
            selection++;
        }

        if (kDown & KEY_A) {
            switch(selection) {
                case 0:
                    state = SORT_SETTINGS;
                    selection = 0;
                    break;

                case 1:
                    cfg.theme = !cfg.theme;
                    Config::Save(cfg);
                    break;
                
                case 2:
                    cfg.debug = !cfg.debug;
                    Config::Save(cfg);
                    break;

                case 3:
                    state = UPDATE_SETTINGS;
                    selection = 0;
                    break;
            }
        }
        else if (kDown & KEY_B) {
            data.state = GUI_STATE_FILEBROWSER;
        }

        if (Touch::Rect(0, 55, 320, 94)) {
            selection = 0;
            
            if (kDown & KEY_TOUCH) {
                state = SORT_SETTINGS;
                selection = 0;
            }
        }
        else if (Touch::Rect(0, 95, 320, 134)) {
            selection = 1;
            
            if (kDown & KEY_TOUCH) {
                cfg.theme = !cfg.theme;
                Config::Save(cfg);
            }
        }
        else if (Touch::Rect(0, 135, 320, 174)) {
            selection = 2;
            
            if (kDown & KEY_TOUCH) {
                cfg.debug = !cfg.debug;
                Config::Save(cfg);
            }
        }
        else if (Touch::Rect(0, 175, 320, 215)) {
            selection = 3;
            
            if (kDown & KEY_TOUCH) {
                state = UPDATE_SETTINGS;
                selection = 0;
            }
        }
        
        Utils::Wrap(selection, 0, 3);
    }

    void DisplaySettings(GuiData& data) {
        GUI::DrawRect(0, 20, 400, 35, guiBgColourBottom[cfg.theme]); // Menu bar
        GUI::DrawRect(0, 55, 320, 185, guiBgColourTop[cfg.theme]);
        if (state != GENERAL_SETTINGS) {
            GUI::DrawImage(iconBack, 5, 25);
        }

        GUI::DrawRect(0, 55 + (selection * guiSelDist), 320, guiSelDist, guiSelectorColour[cfg.theme]);

        switch(state) {
            case GENERAL_SETTINGS:
                GUI::DisplayGeneralSettings();
                break;
            
            case SORT_SETTINGS:
                GUI::DisplaySortSettings();
                break;
            
            case UPDATE_SETTINGS:
                GUI::DisplayUpdateSettings();
                break;
        }
    }

    void ControlSettings(GuiData& data, u32& kDown) {
        switch(state) {
            case GENERAL_SETTINGS:
                GUI::ControlGeneralSettings(data, kDown);
                break;
            
            case SORT_SETTINGS:
                GUI::ControlSortSettings(data, kDown);
                break;
            
            case UPDATE_SETTINGS:
                GUI::ControlUpdateSettings(data, kDown);
                break;
        }
    }
}
