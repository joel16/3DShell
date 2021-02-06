#include <algorithm>
#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    static const int sel_dist = 20;
    static const int start_y = 40;
    static const u32 max_entries = 10;
    static int start = 0;

    static std::string empty_dir = "This is an empty directory";
    static float empty_dir_width = 0.0f, empty_dir_height = 0.0f;

    void DisplayFileBrowser(MenuItem *item) {
        float filename_height = 0.0f;
        C2D::GetTextSize(0.45f, nullptr, &filename_height, cfg.cwd.c_str());
        C2D::Text(5, 15 + ((25 - filename_height) / 2), 0.45f, WHITE, cfg.cwd.c_str());

        // Storage bar
        C2D::Rect(5, 28 + ((25 - filename_height) / 2), 390, 2, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        float fill = (static_cast<double>(item->used_storage)/static_cast<double>(item->total_storage)) * 390.0;
        C2D::Rect(5, 28 + ((25 - filename_height) / 2), fill, 2, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

        if (item->entries.empty()) {
            C2D::GetTextSize(0.50f, &empty_dir_width, &empty_dir_height, empty_dir.c_str());
            C2D::Text(((400 - empty_dir_width) / 2), ((240 - empty_dir_height) / 2), 0.50f, cfg.dark_theme? WHITE : BLACK, empty_dir.c_str());
        }

        for (u32 i = start; i < item->entries.size(); i++) {
            const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item->entries[i].name);
            const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());

            if (i == static_cast<u32>(item->selected))
                C2D::Rect(0, start_y + (sel_dist * (i - start)), 400, sel_dist, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

            if ((item->checked.at(i)) && (!item->checked_cwd.compare(cfg.cwd)))
                C2D::Image(cfg.dark_theme? icon_check_dark : icon_check, 0, start_y + (sel_dist * (i - start)));
            else
                C2D::Image(cfg.dark_theme? icon_uncheck_dark : icon_uncheck, 0, start_y + (sel_dist * (i - start)));

            FileType file_type = FS::GetFileType(filename);
            if (item->entries[i].attributes & FS_ATTRIBUTE_DIRECTORY)
                C2D::Image(cfg.dark_theme? icon_dir_dark : icon_dir, 20, start_y + (sel_dist * (i - start)));
            else
                C2D::Image(file_icons[file_type], 20, start_y + (sel_dist * (i - start)));

            C2D::Text(45, start_y + ((sel_dist - filename_height) / 2) + (i - start) * sel_dist, 0.45f, cfg.dark_theme? WHITE : BLACK, filename.c_str());
        }
    }

    void ControlFileBrowser(MenuItem *item, u32 *kDown) {
        u32 size = (item->entries.size() - 1);
        Utils::SetBounds(&item->selected, 0, size);

        if (*kDown & KEY_DUP) {
            item->selected--;
            if (item->selected < 0)
                item->selected = size;

            if (size < max_entries)
                start = 0;
            else if (start > item->selected)
                start--;
            else if ((static_cast<u32>(item->selected) == size) && (size > (max_entries - 1)))
                start = size - (max_entries - 1);
        }
        else if (*kDown & KEY_DDOWN) {
            item->selected++;
            if(static_cast<u32>(item->selected) > size)
                item->selected = 0;

            if ((static_cast<u32>(item->selected) > (start + (max_entries - 1))) && ((start + (max_entries - 1)) < size))
                start++;
            if (item->selected == 0)
                start = 0;
        }

        else if (*kDown & KEY_A) {
            const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item->entries[item->selected].name);
            const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());

            if (item->entries[item->selected].attributes & FS_ATTRIBUTE_DIRECTORY) {
                if (item->entries.size() != 0) {
                    if (R_SUCCEEDED(FS::ChangeDirNext(filename, item->entries))) {
                        start = 0;
                        // Make a copy before resizing our vector.
                        if ((item->checked_count > 1) && (item->checked_copy.empty()))
                            item->checked_copy = item->checked;
                        
                        item->checked.resize(item->entries.size());
                        item->selected = 0;
                    }
                }
            }
            else {
                std::string path = cfg.cwd;
                path.append(filename);
                FileType file_type = FS::GetFileType(filename);
                
                switch(file_type) {
                    case FileTypeImage:
                        if (Textures::LoadImageFile(path, item->textures))
                            item->state = MENU_STATE_IMAGEVIEWER;
                        break;

                    default:
                        break;
                }
            }
        }
        else if (*kDown & KEY_B) {
            if (R_SUCCEEDED(FS::ChangeDirPrev(item->entries))) {
                // Make a copy before resizing our vector.
                if (item->checked_count > 1)
                    item->checked_copy = item->checked;
                    
                item->checked.resize(item->entries.size());
                item->selected = 0;
            }
        }
        else if (*kDown & KEY_Y) {
            if ((!item->checked_cwd.empty()) && (item->checked_cwd.compare(cfg.cwd) != 0))
                GUI::ResetCheckbox(item);
                
            item->checked_cwd = cfg.cwd;
            item->checked.at(item->selected) = !item->checked.at(item->selected);
            item->checked_count = std::count(item->checked.begin(), item->checked.end(), true);
        }
        else if (*kDown & KEY_X)
            item->state = MENU_STATE_OPTIONS;
    }
}
