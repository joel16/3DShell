#include <algorithm>
#include <codecvt>
#include <locale>

#include "archive_helper.h"
#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "hbloader.h"
#include "netloader.h"
#include "notepad.h"
#include "osk.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {
    static const int sel_dist = 20;
    static const int start_y = 40;
    static const u32 max_entries = 10;
    static int start = 0;
    static u64 timestamp = 0;

    static std::string empty_dir = "This is an empty directory";
    static float empty_dir_width = 0.f, empty_dir_height = 0.f;

    void DisplayFileBrowser(MenuItem *item) {
        float filename_height = 0.f;
        C2D::GetTextSize(0.45f, nullptr, &filename_height, cfg.cwd.c_str());
        C2D::Textf(5, 15 + ((25 - filename_height) / 2), 0.45f, WHITE, cfg.cwd.length() > 60? "%.60s..." : "%s", cfg.cwd.c_str());

        // Storage bar
        C2D::Rect(5, 28 + ((25 - filename_height) / 2), 390, 2, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        float fill = (static_cast<double>(item->used_storage)/static_cast<double>(item->total_storage)) * 390.f;
        C2D::Rect(5, 28 + ((25 - filename_height) / 2), fill, 2, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

        if (item->entries.empty()) {
            C2D::GetTextSize(0.5f, &empty_dir_width, &empty_dir_height, empty_dir.c_str());
            C2D::Text(((400 - empty_dir_width) / 2), ((240 - empty_dir_height) / 2), 0.5f, cfg.dark_theme? WHITE : BLACK, empty_dir.c_str());
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

            C2D::Textf(45, start_y + ((sel_dist - filename_height) / 2) + (i - start) * sel_dist, 0.45f, cfg.dark_theme? WHITE : BLACK,
                filename.length() > 52? "%.52s..." : "%s", filename.c_str());
        }
    }

    void DisplayFileBrowserBottom(void) {
        // Grid of 4 buttons on bottom screen
        const char* button_texts[4] = {"Nuevo Archivo", "Nueva Carpeta", "Propiedades", "Renombrar"};
        static float button_widths[4] = {0};
        static float button_heights[4] = {0};
        static bool sizes_calculated = false;

        if (!sizes_calculated) {
            for (int i = 0; i < 4; i++) {
                C2D::GetTextSize(0.4f, &button_widths[i], &button_heights[i], button_texts[i]);
            }
            sizes_calculated = true;
        }

        // Button positions (2x2 grid)
        float button_width = 140; // Fixed width for each button
        float button_height = 30;
        float margin = 10;
        float start_x = (320 - 2 * button_width - margin) / 2; // Center the grid
        float start_y = 80;

        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
                int index = row * 2 + col;
                float x = start_x + col * (button_width + margin);
                float y = start_y + row * (button_height + margin);

                // Draw button background
                C2D::Rect(x, y, button_width, button_height, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
                
                // Draw button text centered
                float text_x = x + (button_width - button_widths[index]) / 2;
                float text_y = y + (button_height - button_heights[index]) / 2;
                C2D::Text(text_x, text_y, 0.4f, cfg.dark_theme? WHITE : BLACK, button_texts[index]);
            }
        }

        // NetLoader hint (SELECT key), centered below the button grid
        static const char* hint_text = "3ds netloader - <select>";
        float hint_width = 0.f, hint_height = 0.f;
        C2D::GetTextSize(0.4f, &hint_width, &hint_height, hint_text);
        C2D::Text(((320 - hint_width) / 2), 140 + margin, 0.4f, cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, hint_text);
    }

    static void CreateFolder(MenuItem *item) {
        std::string path = cfg.cwd;
        std::string name = OSK::GetText("Nueva Carpeta", "Ingrese nombre de la carpeta");
        if (!name.empty()) {
            path.append(name);
            std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());
            
            if (R_SUCCEEDED(FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0))) {
                FS::GetDirList(cfg.cwd, item->entries);
                GUI::ResetCheckbox(item);
            }
        }
    }

    static void RenameSelected(MenuItem *item) {
        if (item->entries.empty() || item->selected >= item->entries.size()) return;
        
        const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item->entries[item->selected].name);
        const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());
        
        std::string new_name = OSK::GetText(filename, "Ingrese nuevo nombre");
        if (!new_name.empty() && R_SUCCEEDED(FS::Rename(&item->entries[item->selected], new_name.c_str()))) {
            FS::GetDirList(cfg.cwd, item->entries);
            GUI::ResetCheckbox(item);
        }
    }

    static void CreateFile(MenuItem *item) {
        std::string path = cfg.cwd;
        std::string name = OSK::GetText("Nuevo Archivo", "Ingrese nombre del archivo");
        if (!name.empty()) {
            path.append(name);
            std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());
            
            if (R_SUCCEEDED(FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0, 0))) {
                FS::GetDirList(cfg.cwd, item->entries);
                GUI::ResetCheckbox(item);
            }
        }
    }

    void ControlFileBrowserBottom(MenuItem *item, u32 *kDown) {
        if (*kDown & KEY_TOUCH) {
            // Button grid parameters
            float button_width = 140;
            float button_height = 30;
            float margin = 10;
            float start_x = (320 - 2 * button_width - margin) / 2;
            float start_y = 80;

            for (int row = 0; row < 2; row++) {
                for (int col = 0; col < 2; col++) {
                    float x = start_x + col * (button_width + margin);
                    float y = start_y + row * (button_height + margin);

                    if (Touch::Rect(x, y, x + button_width, y + button_height)) {
                        int index = row * 2 + col;
                        switch (index) {
                            case 0: // Nuevo Archivo
                                CreateFile(item);
                                break;
                            case 1: // Nueva Carpeta
                                CreateFolder(item);
                                break;
                            case 2: // Propiedades
                                item->state = MENU_STATE_PROPERTIES;
                                break;
                            case 3: // Renombrar
                                RenameSelected(item);
                                break;
                        }
                        return;
                    }
                }
            }
        }
    }

    void ControlFileBrowser(MenuItem *item, u32 *kDown, u32 *kHeld) {
        u32 size = (item->entries.size() - 1);
        Utils::SetBounds(&item->selected, 0, size);

        if ((*kDown & KEY_UP) || ((*kHeld & KEY_UP) && osGetTime() >= timestamp)) {
            item->selected--;
            if (item->selected < 0)
                item->selected = size;

            if (size < max_entries)
                start = 0;
            else if (start > item->selected)
                start--;
            else if ((static_cast<u32>(item->selected) == size) && (size > (max_entries - 1)))
                start = size - (max_entries - 1);

            timestamp = osGetTime() + ((*kDown & KEY_UP) ? 500 : 100);
        }
        else if ((*kDown & KEY_DOWN) || ((*kHeld & KEY_DOWN) && osGetTime() >= timestamp)) {
            item->selected++;
            if(static_cast<u32>(item->selected) > size)
                item->selected = 0;

            if ((static_cast<u32>(item->selected) > (start + (max_entries - 1))) && ((start + (max_entries - 1)) < size))
                start++;
            if (item->selected == 0)
                start = 0;

            timestamp = osGetTime() + ((*kDown & KEY_DOWN) ? 500 : 100);
        }

        if (*kDown & KEY_DLEFT) {
            item->selected = 0;
            start = 0;
        }
        else if (*kDown & KEY_DRIGHT) {
            item->selected = item->entries.size() - 1;
            if ((item->entries.size() - 1) > max_entries)
                start = size - (max_entries - 1);
        }

        if (*kDown & KEY_A) {
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
                        if (Textures::LoadImageFile(path, &item->texture))
                            item->state = MENU_STATE_IMAGEVIEWER;
                        break;

                    case FileTypeText:
                        if (Notepad::IsTextFile(filename) && Notepad::Open(path, filename))
                            item->state = MENU_STATE_TEXTREADER;
                        break;

                    case FileTypeZip:
                        if (R_SUCCEEDED(ArchiveHelper::Extract(path))) {
                            FS::GetDirList(cfg.cwd, item->entries);
                            GUI::ResetCheckbox(item);
                        }

                        break;

                    case FileType3dsx:
                        HBLoader::Boot(path);
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
                start = 0;
            }
        }
        else if (*kDown & KEY_Y) {
            if ((!item->checked_cwd.empty()) && (item->checked_cwd.compare(cfg.cwd) != 0))
                GUI::ResetCheckbox(item);
                
            item->checked_cwd = cfg.cwd;
            item->checked.at(item->selected) = !item->checked.at(item->selected);
            item->checked_count = std::count(item->checked.begin(), item->checked.end(), true);
        }
        else if (*kDown & KEY_SELECT) {
            if (NetLoader::Run() == NL_NOCHAIN)
                GUI::ProgressBar("NetLoader", "No chainloader available", 0, 1);
        }
        else if (*kDown & KEY_X)
            item->state = MENU_STATE_OPTIONS;
    }
}
