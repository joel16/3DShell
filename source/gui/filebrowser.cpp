#include <algorithm>
#include <cstring>

#include "archiveextractor.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    constexpr int guiSelDist = 20;
    constexpr int guiStartY = 40;
    constexpr int maxEntries = 10;
    static int start = 0;
    static u64 timestamp = 0;

    constexpr u32 guiTextColour[2] = { C2D_Color32(0, 0, 0, 255), C2D_Color32(255, 255, 255, 255) };

    void DisplayFileBrowser(GuiData& data) {
        GUI::DrawTextf(5, 15 + ((25 - data.textHeight) / 2), 0.45f, guiTextColour[1], cfg.cwd.length() > 60? "%.60s..." : "%s", Utils::UTF16ToUTF8(reinterpret_cast<u16 *>(cfg.cwd.data())).c_str());

        // Storage bar
        GUI::DrawRect(5, 28 + ((25 - data.textHeight) / 2), 390, 2, guiTextColour[cfg.theme]);
        float fill = (static_cast<float>(data.usedSize) / data.totalSize) * 390.f;
        GUI::DrawRect(5, 28 + ((25 - data.textHeight) / 2), fill, 2, guiTitleColour[cfg.theme]);

        // Bound the loop so it only processes the max entries (10) that are visible on the screen.
        int end = std::min(static_cast<int>(data.entries.size()), start + maxEntries);
        bool cwdMatch = !data.checkedCwd.compare(cfg.cwd);

        for (int i = start; i < end; i++) {
            char filename[256] = {};
            Utils::UTF16ToUTF8(reinterpret_cast<u8*>(filename), reinterpret_cast<const u16*>(data.entries[i].name), sizeof(filename) - 1);

            if (i == data.selected) {
                GUI::DrawRect(0, guiStartY + (guiSelDist * (i - start)), 400, guiSelDist, guiSelectorColour[cfg.theme]);
            }

            if ((data.checked.at(i)) && (cwdMatch)) {
                GUI::DrawImage(iconCheck[cfg.theme], 0, guiStartY + (guiSelDist * (i - start)));
            }
            else {
                GUI::DrawImage(iconUncheck[cfg.theme], 0, guiStartY + (guiSelDist * (i - start)));
            }

            if (data.entries[i].attributes & FS_ATTRIBUTE_DIRECTORY) {
                GUI::DrawImage(iconDir[cfg.theme], 20, guiStartY + (guiSelDist * (i - start)));
            }
            else {
                FileType fileType = FS::GetFileType(data.entries[i].shortExt);
                GUI::DrawImage(fileIcon[fileType], 20, guiStartY + (guiSelDist * (i - start)));
            }
            
            bool truncated = (filename[52] != '\0');
            GUI::DrawTextf(45, guiStartY + ((guiSelDist - data.textHeight) / 2) + (i - start) * guiSelDist, 0.45f, guiTextColour[cfg.theme],
                truncated? "%.52s..." : "%s", filename);
        }
    }

    void ControlFileBrowser(GuiData& data, u32& kDown, u32& kHeld) {
        int size = (data.entries.size() - 1);
        Utils::Wrap(data.selected, 0, size);

        if ((kDown & KEY_UP) || ((kHeld & KEY_UP) && osGetTime() >= timestamp)) {
            data.selected--;

            if (data.selected < 0) {
                data.selected = size;
            }

            if (size < maxEntries) {
                start = 0;
            }
            else if (start > data.selected) {
                start--;
            }
            else if ((data.selected == size) && (size > (maxEntries - 1))) {
                start = size - (maxEntries - 1);
            }

            timestamp = osGetTime() + ((kDown & KEY_UP) ? 500 : 100);
        }
        else if ((kDown & KEY_DOWN) || ((kHeld & KEY_DOWN) && osGetTime() >= timestamp)) {
            data.selected++;

            if(data.selected > size) {
                data.selected = 0;
            }

            if ((data.selected > (start + (maxEntries - 1))) && ((start + (maxEntries - 1)) < size)) {
                start++;
            }
            if (data.selected == 0) {
                start = 0;
            }

            timestamp = osGetTime() + ((kDown & KEY_DOWN) ? 500 : 100);
        }

        if (kDown & KEY_DLEFT) {
            data.selected = 0;
            start = 0;
        }
        else if (kDown & KEY_DRIGHT) {
            data.selected = data.entries.size() - 1;

            if ((data.entries.size() - 1) > maxEntries) {
                start = size - (maxEntries - 1);
            }
        }
        if (kDown & KEY_A) {
            if (data.entries[data.selected].attributes & FS_ATTRIBUTE_DIRECTORY) {
                if (data.entries.size() != 0) {
                    if (R_SUCCEEDED(FS::ChangeDirNext(reinterpret_cast<const char16_t *>(data.entries[data.selected].name), data.entries))) {
                        start = 0;
                        // Make a copy before resizing our vector.
                        if ((data.checkedCount > 1) && (data.checkedCopy.empty())) {
                            data.checkedCopy = data.checked;
                        }
                        
                        data.checked.resize(data.entries.size());
                        data.selected = 0;
                    }
                }
            }
            else {
                char path[1024];
                FS::GetUTF8Path(path, sizeof(path), data.entries[data.selected].name);
                FileType fileType = FS::GetFileType(data.entries[data.selected].shortExt);

                switch (fileType) {
                    case FileTypeAudio:
                        GUI::DisplayAudioPlayer(path);
                        break;

                    case FileTypeImage:
                        GUI::DisplayImageViewer(path, data);
                        break;

                    case FileTypeArchive:
                        if (R_SUCCEEDED(ArchiveExtractor::Extract(path))) {
                            FS::GetDirList(cfg.cwd, data.entries);
                            GUI::ResetCheckbox(data);
                        }
                        break;
                    
                    default:
                        break;
                }
            }
        }
        else if (kDown & KEY_B) {
            if (R_SUCCEEDED(FS::ChangeDirPrev(data.entries))) {
                // Make a copy before resizing our vector.
                if (data.checkedCount > 1) {
                    data.checkedCopy = data.checked;
                }
                    
                data.checked.resize(data.entries.size());
                data.selected = 0;
                start = 0;
            }
        }
        else if (kDown & KEY_Y) {
            if ((!data.checkedCwd.empty()) && (data.checkedCwd.compare(cfg.cwd) != 0)) {
                GUI::ResetCheckbox(data);
            }
                
            data.checkedCwd = cfg.cwd;
            data.checked.at(data.selected) = !data.checked.at(data.selected);
            data.checkedCount = std::count(data.checked.begin(), data.checked.end(), 1);
        }
        else if (kDown & KEY_X) {
            data.state = GUI_STATE_OPTIONS;
        }
    }
}
