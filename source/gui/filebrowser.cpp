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
#include "smdh.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {
    /* Cached cwd as UTF16, rebuilt on directory changes (control phase; the
     * draw phase only reads it). Declared here so both the top-screen list
     * and the bottom SMDH panel can address entries by full UTF16 path. */
    static std::u16string smdh_cwd_u16;

    /* Last-session position restore runs once per GUI::Loop lifetime. */
    static bool browser_position_restored = false;

    /* UTF16 strlen (entries carry fixed 262-char name arrays). */
    static std::size_t u16len(const u16 *src) {
        std::size_t len = 0;
        while (src[len])
            len++;
        return len;
    }

    static const int sel_dist = 20;
    static const int start_y = 40;
    static const u32 max_entries = 10;
    static int start = 0;

    /* Entry display cache: names + FileTypes computed once per directory
     * listing (invalidated against FS::dirlist_epoch) so the draw loop
     * never touches std::codecvt or FS per row. */
    static u32 cache_epoch = 0;
    static std::size_t cache_count = 0;
    static std::vector<std::string> cache_names;
    static std::vector<FileType> cache_types;
    static u64 timestamp = 0;

    /* Per-directory navigation anchors: pushing the current directory's
     * position before entering a subdirectory lets Back (B) restore the
     * exact spot in the parent, and re-entering any sibling refreshes
     * its anchor. One anchor per directory, capped. */
    typedef struct BrowserAnchor {
        std::string cwd;
        int selected;
        int scroll;
    } BrowserAnchor;

    static std::vector<BrowserAnchor> nav_anchors;

    static void PushNavAnchor(MenuItem *item) {
        std::string cwd = cfg.cwd;

        /* Keep at most one anchor per directory (the freshest). */
        for (std::size_t i = 0; i < nav_anchors.size();) {
            if (nav_anchors[i].cwd.compare(cwd) == 0)
                nav_anchors.erase(nav_anchors.begin() + i);
            else
                i++;
        }

        BrowserAnchor anchor;
        anchor.cwd = cwd;
        anchor.selected = item->selected;
        anchor.scroll = start;
        nav_anchors.push_back(anchor);

        if (nav_anchors.size() > 64)
            nav_anchors.erase(nav_anchors.begin());
    }

    /* Restore data for the newest anchor of a directory (if any); used
     * when navigating back (B) so the parent loads where you left it.
     * Selection validity is checked by the caller against the loaded
     * directory list. */
    static int pending_anchor_selected = 0;
    static int pending_anchor_scroll = -1;

    static bool PeekNavAnchor(const std::string &cwd) {
        for (std::size_t i = nav_anchors.size(); i-- > 0;) {
            if (nav_anchors[i].cwd.compare(cwd) == 0) {
                pending_anchor_selected = nav_anchors[i].selected;
                pending_anchor_scroll = nav_anchors[i].scroll;
                return true;
            }
        }
        pending_anchor_scroll = -1;
        return false;
    }

    static std::string empty_dir = "This is an empty directory";
    static float empty_dir_width = 0.f, empty_dir_height = 0.f;

    /* Clamp the scroll window for a freshly (re)loaded directory. */
    static void setBrowserStart(MenuItem *item) {
        int max_start = (int)item->entries.size() - static_cast<int>(max_entries);
        if (max_start < 0)
            max_start = 0;
        if (start < 0)
            start = 0;
        start = std::min(start, max_start);

        /* Keep the selection visible inside the window. */
        if ((item->selected < start) || (item->selected >= (start + (int)max_entries))) {
            start = std::max(0, item->selected - (int)max_entries + 1);
        }
    }

    /* Last-session position restore runs once per GUI::Loop lifetime. */
    void RestoreLastBrowserPosition(MenuItem *item) {
        if (browser_position_restored)
            return;

        browser_position_restored = true;

        if ((item->entries.empty()) || (cfg.last_selected >= static_cast<int>(item->entries.size())))
            return;

        int max_start = (int)item->entries.size() - static_cast<int>(max_entries);
        if (max_start < 0)
            max_start = 0;

        item->selected = cfg.last_selected;
        start = (cfg.last_scroll < 0)? 0 : std::min(cfg.last_scroll, max_start);
    }

    /* Mirror the live position into the config cache (cheap; Config::Save
     * only happens on directory changes and app exit). */
    void SaveBrowserPosition(MenuItem *item) {
        cfg.last_selected = item->selected;
        cfg.last_scroll = start;
    }

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

        int end = std::min(static_cast<int>(item->entries.size()), start + static_cast<int>(max_entries));
        bool cwd_match = (item->checked_cwd.compare(cfg.cwd) == 0);

        for (int i = start; i < end; i++) {
            /* The cache is built in the control phase; the very first
             * frame after boot (or a huge listing refresh) can render
             * before it filled — draw nothing for those rows instead
             * of indexing an empty vector. */
            if (i >= static_cast<int>(cache_names.size()))
                continue;

            const std::string &filename = cache_names[i];

            if (i == item->selected)
                C2D::Rect(0, start_y + (sel_dist * (i - start)), 400, sel_dist, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

            if ((item->checked.at(i)) && cwd_match)
                C2D::Image(cfg.dark_theme? icon_check_dark : icon_check, 0, start_y + (sel_dist * (i - start)));
            else
                C2D::Image(cfg.dark_theme? icon_uncheck_dark : icon_uncheck, 0, start_y + (sel_dist * (i - start)));

            if (item->entries[i].attributes & FS_ATTRIBUTE_DIRECTORY)
                C2D::Image(cfg.dark_theme? icon_dir_dark : icon_dir, 20, start_y + (sel_dist * (i - start)));
            else if ((i < static_cast<int>(cache_types.size())) && (cache_types[i] == FileType3dsx)) {
                /* SMDH icon scaled to the list row (like hbmenu tiles, 48->16). */
                SMDHMeta *meta = nullptr;
                std::u16string path = smdh_cwd_u16;
                const char16_t *entry_name16 = reinterpret_cast<const char16_t *>(item->entries[i].name);
                path.append(entry_name16, u16len(item->entries[i].name));

                if (SMDH::Peek(path, &meta) && meta->has_icon)
                    C2D::ImageScale(meta->icon, 20, start_y + (sel_dist * (i - start)) + 2, 16.0f / 48.0f, 16.0f / 48.0f);
                else
                    C2D::Image(file_icons[FileTypeNone], 20, start_y + (sel_dist * (i - start)));
            }
            else if ((i < static_cast<int>(cache_types.size())) && (cache_types[i] >= FileTypeNone) && (cache_types[i] <= FileType3dsx))
                C2D::Image(file_icons[cache_types[i]], 20, start_y + (sel_dist * (i - start)));

            C2D::Textf(45, start_y + ((sel_dist - filename_height) / 2) + (i - start) * sel_dist, 0.45f, cfg.dark_theme? WHITE : BLACK,
                filename.length() > 52? "%.52s..." : "%s", filename.c_str());
        }
    }

    /* Chop text to fit max_width (UTF8-aware), append ".." when cut. */
    static std::string FitText(const std::string &text, float size, float max_width) {
        if (text.empty())
            return std::string();

        std::string out = text;
        float width = 0.f;
        C2D::GetTextSize(size, &width, nullptr, out.c_str());
        while ((width > max_width) && (out.length() > 1)) {
            std::size_t cut = out.length() - 1;
            while (cut && ((out[cut] & 0xC0) == 0x80)) /* keep UTF8 multibyte atoms */
                cut--;
            if (cut == 0)
                cut = 1;
            out.resize(out.length() - (out.length() - cut));

            out.append("..");
            C2D::GetTextSize(size, &width, nullptr, out.c_str());
            if (width <= max_width)
                break;
            out.erase(out.length() - 2); /* still too wide: keep chopping */
        }
        return out;
    }

    /* UTF16 (FS names) -> UTF8, defensive: never abort on malformed names.
     * Surrogates are dropped (exceptions are disabled in devkitARM). */
    static std::string SafeUtf16ToUtf8(const char16_t *src, std::size_t len) {
        std::u16string clean;
        for (std::size_t i = 0; i < len; i++) {
            char16_t c = src[i];
            if ((c >= 0xD800) && (c <= 0xDFFF))
                continue;
            clean.push_back(c);
        }

        return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(clean);
    }

    static void ControlEntryCache(MenuItem *item) {
        if ((cache_epoch == FS::dirlist_epoch) && (cache_count == item->entries.size()))
            return;

        cache_names.clear();
        cache_types.clear();
        cache_names.reserve(item->entries.size());
        cache_types.reserve(item->entries.size());

        for (const FS_DirectoryEntry &entry : item->entries) {
            cache_names.push_back(SafeUtf16ToUtf8(reinterpret_cast<const char16_t *>(entry.name), u16len(entry.name)));
            cache_types.push_back((entry.attributes & FS_ATTRIBUTE_DIRECTORY)?
                FileTypeNone : FS::GetFileType(cache_names.back()));
        }

        cache_epoch = FS::dirlist_epoch;
        cache_count = item->entries.size();
    }



    /* ASCII-safe utf8 -> utf16 path builder for SMDH requests: cwd strings
     * come from config json; dropping non-ASCII bytes beats aborting the
     * whole app on a malformed cwd (devkitARM builds have no exceptions). */
    static std::u16string ToUtf16Path(const std::string &path) {
        std::u16string clean;
        for (std::size_t i = 0; i < path.length(); i++) {
            unsigned char c = (unsigned char)path[i];
            if (c & 0x80)
                continue;
            clean.push_back((char16_t)c);
        }
        return clean;
    }

    /* SMDH decode trigger — runs in the control phase (outside the C3D frame).
     * Feeds the whole visible window (plus selection) to the one-decode-per-
     * frame LRU pump. */
    static void ControlSmdhPanel(MenuItem *item) {
        smdh_cwd_u16 = ToUtf16Path(cfg.cwd);
        if ((item->entries.empty()) ||
            (item->selected >= static_cast<int>(item->entries.size())))
            return;

        std::vector<std::u16string> wanted;

        u32 last = (item->entries.size() < (u32)(start + max_entries))?
            item->entries.size() : (u32)(start + max_entries);
        for (u32 i = start; i < last; i++) {
            if ((item->entries[i].attributes & FS_ATTRIBUTE_DIRECTORY) || (cache_types[i] != FileType3dsx))
                continue;

            std::u16string path = smdh_cwd_u16;
            const char16_t *name16 = reinterpret_cast<const char16_t *>(item->entries[i].name);
            path.append(name16, u16len(item->entries[i].name));

            wanted.push_back(path);
        }

        /* Always keep the selected entry decoded for the bottom panel. */
        std::u16string sel_path = smdh_cwd_u16;
        const char16_t *sel_name16 = reinterpret_cast<const char16_t *>(item->entries[item->selected].name);
        sel_path.append(sel_name16, u16len(item->entries[item->selected].name));
        if (!(item->entries[item->selected].attributes & FS_ATTRIBUTE_DIRECTORY) &&
            (cache_types[item->selected] == FileType3dsx)) {
            bool listed = false;
            for (const std::u16string &p : wanted)
                if (p.compare(sel_path) == 0)
                    listed = true;
            if (!listed)
                wanted.push_back(sel_path);
        }

        if (!wanted.empty())
            SMDH::Pump(wanted);
    }

    /* SMDH info panel for the selected .3dsx entry (icon + name/desc/publisher). */
    static void DisplaySmdhPanel(MenuItem *item) {
        if ((item->entries.empty()) || smdh_cwd_u16.empty() ||
            (item->selected >= static_cast<int>(item->entries.size())))
            return;

        const std::u16string entry_name(
            reinterpret_cast<const char16_t *>(item->entries[item->selected].name),
            u16len(item->entries[item->selected].name));
        if (FS::GetFileType(SafeUtf16ToUtf8(entry_name.data(), entry_name.size())) != FileType3dsx)
            return;

        SMDHMeta *meta = nullptr;
        if (!SMDH::Peek(smdh_cwd_u16 + entry_name, &meta))
            return;

        u32 dim = cfg.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT;
        float x = 16;

        if (meta->has_icon)
            C2D::Image(meta->icon, x, 26);

        if (!meta->name.empty())
            C2D::Text((meta->has_icon? x + 56 : x), 26, 0.45f, cfg.dark_theme? WHITE : BLACK, FitText(SafeUtf16ToUtf8(meta->name.data(), meta->name.size()), 0.45f, 232).c_str());

        if (!meta->description.empty())
            C2D::Text((meta->has_icon? x + 56 : x), 44, 0.35f, dim, FitText(meta->description, 0.35f, 232).c_str());

        if (!meta->publisher.empty())
            C2D::Text((meta->has_icon? x + 56 : x), 58, 0.35f, dim, FitText(meta->publisher, 0.35f, 232).c_str());
    }

    void DisplayFileBrowserBottom(MenuItem *item) {
        DisplaySmdhPanel(item);

        // Grid of 4 buttons on bottom screen
        const char* button_texts[5] = {"Nuevo Archivo", "Nueva Carpeta", "Propiedades", "Renombrar", "NetLoader"};
        static float button_widths[5] = {0};
        static float button_heights[5] = {0};
        static bool sizes_calculated = false;

        if (!sizes_calculated) {
            for (int i = 0; i < 5; i++) {
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

        // NetLoader button below the 2x2 grid, full-grid width; SELECT also opens it
        float netloader_x = start_x;
        float netloader_y = start_y + 2 * (button_height + margin);
        C2D::Rect(netloader_x, netloader_y, (2 * button_width + margin), button_height, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        C2D::Text(netloader_x + ((2 * button_width + margin) - button_widths[4]) / 2,
            netloader_y + (button_height - button_heights[4]) / 2, 0.4f, cfg.dark_theme? WHITE : BLACK, button_texts[4]);
    }

    static void RunNetLoader(void) {
        if (NetLoader::Run() == NL_NOCHAIN)
            GUI::ProgressBar("NetLoader", "No chainloader available", 0, 1);
    }

    static void CreateFolder(MenuItem *item) {
        std::string path = cfg.cwd;
        std::string name = OSK::GetText("Nueva Carpeta", "Ingrese nombre de la carpeta");
        if (!name.empty()) {
            path.append(name);
            std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());
            
            if (R_SUCCEEDED(FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0))) {
                FS::GetDirList(cfg.cwd, item->entries, cfg.filter);
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
            FS::GetDirList(cfg.cwd, item->entries, cfg.filter);
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
                FS::GetDirList(cfg.cwd, item->entries, cfg.filter);
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

            // NetLoader button (below the grid, full-grid width)
            float netloader_x = start_x;
            float netloader_y = start_y + 2 * (button_height + margin);
            if (Touch::Rect(netloader_x, netloader_y, netloader_x + (2 * button_width + margin),
                netloader_y + button_height)) {
                RunNetLoader();
            }
        }
    }

    void ControlFileBrowser(MenuItem *item, u32 *kDown, u32 *kHeld) {
        /* Both run in the control phase (outside the C3D frame): the cache
         * rebuild (names + FileTypes, one pass per directory listing) and
         * the SMDH decode budget. */
        ControlEntryCache(item);
        ControlSmdhPanel(item);

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
                    /* Anchor the current position before entering, so
                     * Back (B) returns exactly here. */
                    PushNavAnchor(item);

                    if (R_SUCCEEDED(FS::ChangeDirNext(filename, item->entries))) {
                        start = 0;
                        // Make a copy before resizing our vector.
                        if ((item->checked_count > 1) && (item->checked_copy.empty()))
                            item->checked_copy = item->checked;
                        
                        item->checked.resize(item->entries.size());
                        item->selected = 0;

                        /* Fresh position for the new directory. */
                        cfg.last_selected = 0;
                        cfg.last_scroll = 0;
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
                            FS::GetDirList(cfg.cwd, item->entries, cfg.filter);
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
            /* Settle the child's live position before going up; the config
             * snapshot of this session happens on app exit. */
            GUI::SaveBrowserPosition(item);

            if (R_SUCCEEDED(FS::ChangeDirPrev(item->entries))) {
                // Make a copy before resizing our vector.
                if (item->checked_count > 1)
                    item->checked_copy = item->checked;

                item->checked.resize(item->entries.size());

                /* Re-anchor to how this directory was left by entering it
                 * (fall back to its top when there's no anchor). */
                bool restored = PeekNavAnchor(cfg.cwd);
                if (restored) {
                    item->selected = pending_anchor_selected;
                    if ((item->selected < 0) ||
                        (item->selected >= static_cast<int>(item->entries.size())))
                        item->selected = 0;
                    setBrowserStart(item);
                } else {
                    item->selected = 0;
                    setBrowserStart(item);
                }

                cfg.last_selected = item->selected;
                cfg.last_scroll = start;
            }
        }
        else if (*kDown & KEY_Y) {
            if ((!item->checked_cwd.empty()) && (item->checked_cwd.compare(cfg.cwd) != 0))
                GUI::ResetCheckbox(item);
                
            item->checked_cwd = cfg.cwd;
            item->checked.at(item->selected) = !item->checked.at(item->selected);
            item->checked_count = std::count(item->checked.begin(), item->checked.end(), true);
        }
        else if (*kDown & KEY_SELECT)
            RunNetLoader();
        else if (*kDown & KEY_X)
            item->state = MENU_STATE_OPTIONS;

        /* Mirror the live position into the config cache every control
         * frame; Config::Save only happens on dir changes / app exit. */
        GUI::SaveBrowserPosition(item);
    }
}
