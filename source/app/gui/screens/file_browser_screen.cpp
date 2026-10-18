#include "file_browser_screen.h"

#include <algorithm>
#include <codecvt>
#include <locale>

#include "archive_helper.h"
#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "delete_screen.h"
#include "fs.h"
#include "hbloader.h"
#include "image_viewer_screen.h"
#include "netloader.h"
#include "notepad.h"
#include "options_screen.h"
#include "osk.h"
#include "properties_screen.h"
#include "settings_screen.h"
#include "smdh.h"
#include "text_reader_screen.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {

    static const int sel_dist = 20;
    static const int list_start_y = 40;
    static const u32 max_entries = 10;
    static const char *empty_dir = "This is an empty directory";

    /* UTF16 strlen (entries carry fixed 262-char name arrays). */
    std::size_t FileBrowserScreen::u16len(const u16 *src) {
        std::size_t len = 0;
        while (src[len])
            len++;
        return len;
    }

    FileBrowserScreen::FileBrowserScreen()
        : scroll_bar_(UI::Rect{395.0f, 40.0f, 4.0f, 200.0f}, BAR_COLOUR, TITLE_COLOUR) {
        SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
        SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);

        touch_bar_.Home().Click = [this]() {
            if (dialog_open_) {
                dialog_open_ = false;
                touch_bar_.SetSection(TouchBar::SECTION_BROWSER);
                UI::ScreenManager::Instance().Pop();
            }
        };

        touch_bar_.Options().Click = [this]() {
            if (dialog_open_) {
                dialog_open_ = false;
                UI::ScreenManager::Instance().Pop();
            }
            touch_bar_.SetSection(TouchBar::SECTION_ACTIONS);
            PushOptions();
        };

        touch_bar_.Settings().Click = [this]() {
            if (dialog_open_) {
                dialog_open_ = false;
                UI::ScreenManager::Instance().Pop();
            }
            touch_bar_.SetSection(TouchBar::SECTION_SETTINGS);
            PushSettings();
        };

        touch_bar_.SD().Click = [this]() {
            if (archive != sdmc_archive) {
                archive = sdmc_archive;
                cfg.cwd = "/";
                item_.selected = 0;
                RefreshListing();
                RecalcStorageSize(&item_);
            }
        };

        touch_bar_.Secure().Click = [this]() {
            if ((archive != nand_archive) && (cfg.dev_options)) {
                archive = nand_archive;
                cfg.cwd = "/";
                item_.selected = 0;
                RefreshListing();
                RecalcStorageSize(&item_);
            }
        };

        touch_bar_.Search().Click = [this]() {
            std::string path = OSK::GetText("/", "Enter file path");
            path.append((path.back() != '/')? "/" : "");
            if (FS::DirExists(archive, path)) {
                cfg.cwd = path;
                item_.selected = 0;
                RefreshListing();
                RecalcStorageSize(&item_);
            }
        };
    }

    FileBrowserScreen::~FileBrowserScreen() {
        item_.entries.clear();

        /* Persist the browse position for the next session. */
        SaveBrowserPosition();
        if (archive == sdmc_archive)
            Config::Save(cfg);
    }

    Result FileBrowserScreen::Init() {
        Result ret = FS::GetDirList(cfg.cwd, item_.entries, cfg.filter);
        if (R_FAILED(ret))
            return ret;

        ResetCheckbox(&item_);
        RecalcStorageSize(&item_);
        RestoreLastBrowserPosition();
        return 0;
    }

    void FileBrowserScreen::OnFrame(u64 delta_time) {
        (void)delta_time;

        SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
        SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);

        if (HBLoader::PendingExit()) {
            UI::ScreenManager::Instance().Exit();
            return;
        }

        /* A dialog just popped back to us: re-clamp the window in case the
         * listing changed (deletes, pastes, archives extracted), and
         * re-list if the file filter was toggled while we were away. */
        if (dialog_open_ && UI::ScreenManager::Instance().IsCurrent(this)) {
            dialog_open_ = false;
            touch_bar_.SetSection(TouchBar::SECTION_BROWSER);
            if (pushed_filter_ != cfg.filter)
                RefreshListing();
            SetBrowserStart();
        }
    }

    void FileBrowserScreen::OpenDialog(std::unique_ptr<UI::Screen> screen) {
        pushed_filter_ = cfg.filter;
        dialog_open_ = true;
        UI::ScreenManager::Instance().Push(std::move(screen));
    }

    std::string FileBrowserScreen::SafeUtf16ToUtf8(const char16_t *src, std::size_t len) {
        std::u16string clean;
        for (std::size_t i = 0; i < len; i++) {
            char16_t c = src[i];
            if ((c >= 0xD800) && (c <= 0xDFFF))
                continue;
            clean.push_back(c);
        }

        return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(clean);
    }

    std::u16string FileBrowserScreen::ToUtf16Path(const std::string &path) {
        std::u16string clean;
        for (std::size_t i = 0; i < path.length(); i++) {
            unsigned char c = (unsigned char)path[i];
            if (c & 0x80)
                continue;
            clean.push_back((char16_t)c);
        }
        return clean;
    }

    std::string FileBrowserScreen::FitText(const std::string &text, float size, float max_width) {
        if (text.empty())
            return std::string();

        std::string out = text;
        float width = 0.0f;
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

    std::string FileBrowserScreen::EntryNameUtf8() {
        return SafeUtf16ToUtf8(reinterpret_cast<const char16_t *>(item_.entries[item_.selected].name),
            u16len(item_.entries[item_.selected].name));
    }

    void FileBrowserScreen::PushNavAnchor() {
        std::string cwd = cfg.cwd;

        /* Keep at most one anchor per directory (the freshest). */
        for (std::size_t i = 0; i < nav_anchors_.size();) {
            if (nav_anchors_[i].cwd.compare(cwd) == 0)
                nav_anchors_.erase(nav_anchors_.begin() + i);
            else
                i++;
        }

        BrowserAnchor anchor;
        anchor.cwd = cwd;
        anchor.selected = item_.selected;
        anchor.scroll = start_;
        nav_anchors_.push_back(anchor);

        if (nav_anchors_.size() > 64)
            nav_anchors_.erase(nav_anchors_.begin());
    }

    bool FileBrowserScreen::PeekNavAnchor(const std::string &cwd) {
        for (std::size_t i = nav_anchors_.size(); i-- > 0;) {
            if (nav_anchors_[i].cwd.compare(cwd) == 0) {
                pending_anchor_selected_ = nav_anchors_[i].selected;
                pending_anchor_scroll_ = nav_anchors_[i].scroll;
                return true;
            }
        }
        pending_anchor_scroll_ = -1;
        return false;
    }

    /* Clamp the scroll window for a freshly (re)loaded directory. */
    void FileBrowserScreen::SetBrowserStart() {
        int max_start = (int)item_.entries.size() - static_cast<int>(max_entries);
        if (max_start < 0)
            max_start = 0;
        if (start_ < 0)
            start_ = 0;
        start_ = std::min(start_, max_start);

        /* Keep the selection visible inside the window. */
        if ((item_.selected < start_) || (item_.selected >= (start_ + (int)max_entries))) {
            start_ = std::max(0, item_.selected - (int)max_entries + 1);
        }
    }

    void FileBrowserScreen::RestoreLastBrowserPosition() {
        if ((item_.entries.empty()) || (cfg.last_selected >= static_cast<int>(item_.entries.size())))
            return;

        int max_start = (int)item_.entries.size() - static_cast<int>(max_entries);
        if (max_start < 0)
            max_start = 0;

        item_.selected = cfg.last_selected;
        start_ = (cfg.last_scroll < 0)? 0 : std::min(cfg.last_scroll, max_start);
    }

    /* Mirror the live position into the config cache (cheap; Config::Save
     * only happens on directory changes and app exit). */
    void FileBrowserScreen::SaveBrowserPosition() {
        cfg.last_selected = item_.selected;
        cfg.last_scroll = start_;
    }

    void FileBrowserScreen::ControlEntryCache() {
        if ((cache_epoch_ == FS::dirlist_epoch) && (cache_count_ == item_.entries.size()))
            return;

        cache_names_.clear();
        cache_types_.clear();
        cache_names_.reserve(item_.entries.size());
        cache_types_.reserve(item_.entries.size());

        for (const FS_DirectoryEntry &entry : item_.entries) {
            cache_names_.push_back(SafeUtf16ToUtf8(reinterpret_cast<const char16_t *>(entry.name), u16len(entry.name)));
            cache_types_.push_back((entry.attributes & FS_ATTRIBUTE_DIRECTORY)?
                FileTypeNone : FS::GetFileType(cache_names_.back()));
        }

        cache_epoch_ = FS::dirlist_epoch;
        cache_count_ = item_.entries.size();
    }

    void FileBrowserScreen::ControlSmdhPanel() {
        smdh_cwd_u16_ = ToUtf16Path(cfg.cwd);
        if ((item_.entries.empty()) ||
            (item_.selected >= static_cast<int>(item_.entries.size())))
            return;

        std::vector<std::u16string> wanted;

        u32 last = (item_.entries.size() < (u32)(start_ + max_entries))?
            item_.entries.size() : (u32)(start_ + max_entries);
        for (u32 i = start_; i < last; i++) {
            if ((item_.entries[i].attributes & FS_ATTRIBUTE_DIRECTORY) || (cache_types_[i] != FileType3dsx))
                continue;

            std::u16string path = smdh_cwd_u16_;
            const char16_t *name16 = reinterpret_cast<const char16_t *>(item_.entries[i].name);
            path.append(name16, u16len(item_.entries[i].name));

            wanted.push_back(path);
        }

        /* Always keep the selected entry decoded for the bottom panel. */
        std::u16string sel_path = smdh_cwd_u16_;
        const char16_t *sel_name16 = reinterpret_cast<const char16_t *>(item_.entries[item_.selected].name);
        sel_path.append(sel_name16, u16len(item_.entries[item_.selected].name));
        if (!(item_.entries[item_.selected].attributes & FS_ATTRIBUTE_DIRECTORY) &&
            (cache_types_[item_.selected] == FileType3dsx)) {
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
    void FileBrowserScreen::DisplaySmdhPanel() {
        if ((item_.entries.empty()) || smdh_cwd_u16_.empty() ||
            (item_.selected >= static_cast<int>(item_.entries.size())))
            return;

        const std::u16string entry_name(
            reinterpret_cast<const char16_t *>(item_.entries[item_.selected].name),
            u16len(item_.entries[item_.selected].name));
        if (FS::GetFileType(SafeUtf16ToUtf8(entry_name.data(), entry_name.size())) != FileType3dsx)
            return;

        SMDHMeta *meta = nullptr;
        if (!SMDH::Peek(smdh_cwd_u16_ + entry_name, &meta))
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

    void FileBrowserScreen::DisplayActionGrid() {
        const char *button_texts[5] = {"New File", "New Folder", "Properties", "Rename", "NetLoader"};
        static float button_widths[5] = {0};
        static float button_heights[5] = {0};
        static bool sizes_calculated = false;

        if (!sizes_calculated) {
            for (int i = 0; i < 5; i++)
                C2D::GetTextSize(0.4f, &button_widths[i], &button_heights[i], button_texts[i]);
            sizes_calculated = true;
        }

        float button_width = 140;
        float button_height = 30;
        float margin = 10;
        float grid_x = (320 - 2 * button_width - margin) / 2;
        float grid_y = 80;

        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
                int index = row * 2 + col;
                float x = grid_x + col * (button_width + margin);
                float y = grid_y + row * (button_height + margin);

                C2D::Rect(x, y, button_width, button_height, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

                float text_x = x + (button_width - button_widths[index]) / 2;
                float text_y = y + (button_height - button_heights[index]) / 2;
                C2D::Text(text_x, text_y, 0.4f, cfg.dark_theme? WHITE : BLACK, button_texts[index]);
            }
        }

        float netloader_y = grid_y + 2 * (button_height + margin);
        C2D::Rect(grid_x, netloader_y, (2 * button_width + margin), button_height, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        C2D::Text(grid_x + ((2 * button_width + margin) - button_widths[4]) / 2,
            netloader_y + (button_height - button_heights[4]) / 2, 0.4f, cfg.dark_theme? WHITE : BLACK, button_texts[4]);
    }

    void FileBrowserScreen::DrawTop() {
        status_bar_.Draw();

        C2D::Rect(0, 15, 400, 25, cfg.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);

        float filename_height = 0.0f;
        C2D::GetTextSize(0.45f, nullptr, &filename_height, cfg.cwd.c_str());
        C2D::Textf(5, 15 + ((25 - filename_height) / 2), 0.45f, WHITE, cfg.cwd.length() > 60? "%.60s..." : "%s", cfg.cwd.c_str());

        // Storage bar
        C2D::Rect(5, 28 + ((25 - filename_height) / 2), 390, 2, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        float fill = (static_cast<double>(item_.used_storage) / static_cast<double>(item_.total_storage)) * 390.0f;
        C2D::Rect(5, 28 + ((25 - filename_height) / 2), fill, 2, cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

        if (item_.entries.empty()) {
            float empty_dir_width = 0.0f, empty_dir_height = 0.0f;
            C2D::GetTextSize(0.5f, &empty_dir_width, &empty_dir_height, empty_dir);
            C2D::Text(((400 - empty_dir_width) / 2), ((240 - empty_dir_height) / 2), 0.5f, cfg.dark_theme? WHITE : BLACK, empty_dir);
        }

        int end = std::min(static_cast<int>(item_.entries.size()), start_ + static_cast<int>(max_entries));
        bool cwd_match = (item_.checked_cwd.compare(cfg.cwd) == 0);

        for (int i = start_; i < end; i++) {
            /* The cache is built in the control phase; the very first
             * frame after boot (or a huge listing refresh) can render
             * before it filled — draw nothing for those rows instead
             * of indexing an empty vector. */
            if (i >= static_cast<int>(cache_names_.size()))
                continue;

            const std::string &filename = cache_names_[i];

            if (i == item_.selected)
                C2D::Rect(0, list_start_y + (sel_dist * (i - start_)), 395, sel_dist, cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

            if ((item_.checked.at(i)) && cwd_match)
                C2D::Image(cfg.dark_theme? icon_check_dark : icon_check, 0, list_start_y + (sel_dist * (i - start_)));
            else
                C2D::Image(cfg.dark_theme? icon_uncheck_dark : icon_uncheck, 0, list_start_y + (sel_dist * (i - start_)));

            if (item_.entries[i].attributes & FS_ATTRIBUTE_DIRECTORY)
                C2D::Image(cfg.dark_theme? icon_dir_dark : icon_dir, 20, list_start_y + (sel_dist * (i - start_)));
            else if ((i < static_cast<int>(cache_types_.size())) && (cache_types_[i] == FileType3dsx)) {
                /* SMDH icon scaled to the list row (like hbmenu tiles, 48->16). */
                SMDHMeta *meta = nullptr;
                std::u16string path = smdh_cwd_u16_;
                const char16_t *entry_name16 = reinterpret_cast<const char16_t *>(item_.entries[i].name);
                path.append(entry_name16, u16len(item_.entries[i].name));

                if (SMDH::Peek(path, &meta) && meta->has_icon)
                    C2D::ImageScale(meta->icon, 20, list_start_y + (sel_dist * (i - start_)) + 2, 16.0f / 48.0f, 16.0f / 48.0f);
                else
                    C2D::Image(file_icons[FileTypeNone], 20, list_start_y + (sel_dist * (i - start_)));
            }
            else if ((i < static_cast<int>(cache_types_.size())) && (cache_types_[i] >= FileTypeNone) && (cache_types_[i] <= FileType3dsx))
                C2D::Image(file_icons[cache_types_[i]], 20, list_start_y + (sel_dist * (i - start_)));

            C2D::Textf(45, list_start_y + ((sel_dist - filename_height) / 2) + (i - start_) * sel_dist, 0.45f, cfg.dark_theme? WHITE : BLACK,
                filename.length() > 52? "%.52s..." : "%s", filename.c_str());
        }

        /* Scroll position indicator on the right edge of the list. */
        scroll_bar_.SetRange(static_cast<int>(item_.entries.size()), max_entries);
        scroll_bar_.SetValue(start_);
        scroll_bar_.Draw();
    }

    void FileBrowserScreen::DrawBottom() {
        touch_bar_.Draw();

        /* While a dialog is up, the browser grid stays hidden behind it. */
        if (dialog_open_)
            return;

        DisplaySmdhPanel();
        DisplayActionGrid();
    }

    void FileBrowserScreen::OnKeyHeld(u32 k_held) {
        k_held_ = k_held;
        Touch::Update();
    }

    void FileBrowserScreen::RunNetLoader() {
        if (NetLoader::Run() == NL_NOCHAIN)
            GUI::ProgressBar("NetLoader", "No chainloader available", 0, 1);
    }

    void FileBrowserScreen::RefreshListing() {
        FS::GetDirList(cfg.cwd, item_.entries, cfg.filter);
        ResetCheckbox(&item_);
    }

    void FileBrowserScreen::CreateFolder() {
        std::string path = cfg.cwd;
        std::string name = OSK::GetText("New Folder", "Enter folder name");
        if (!name.empty()) {
            path.append(name);
            std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());

            if (R_SUCCEEDED(FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0)))
                RefreshListing();
        }
    }

    void FileBrowserScreen::RenameSelected() {
        if (item_.entries.empty() || item_.selected >= item_.entries.size()) return;

        std::string new_name = OSK::GetText(EntryNameUtf8(), "Enter new name");
        if (!new_name.empty() && R_SUCCEEDED(FS::Rename(&item_.entries[item_.selected], new_name.c_str())))
            RefreshListing();
    }

    void FileBrowserScreen::CreateFile() {
        std::string path = cfg.cwd;
        std::string name = OSK::GetText("New File", "Enter file name");
        if (!name.empty()) {
            path.append(name);
            std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());

            if (R_SUCCEEDED(FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0, 0)))
                RefreshListing();
        }
    }

    void FileBrowserScreen::PushOptions() {
        touch_bar_.SetSection(TouchBar::SECTION_ACTIONS);
        OpenDialog(std::make_unique<OptionsScreen>(this, &item_));
    }

    void FileBrowserScreen::PushSettings() {
        touch_bar_.SetSection(TouchBar::SECTION_SETTINGS);
        OpenDialog(std::make_unique<SettingsScreen>(this, &item_));
    }

    void FileBrowserScreen::OnKeyDown(u32 k_down) {
        if (k_down & KEY_START) {
            UI::ScreenManager::Instance().Exit();
            return;
        }

        /* Both run in the control phase (outside the C3D frame): the cache
         * rebuild (names + FileTypes, one pass per directory listing) and
         * the SMDH decode budget. */
        ControlEntryCache();
        ControlSmdhPanel();

        u32 size = (item_.entries.size() - 1);
        Utils::SetBounds(&item_.selected, 0, size);

        if ((k_down & KEY_UP) || ((k_held_ & KEY_UP) && osGetTime() >= key_repeat_timestamp_)) {
            item_.selected--;
            if (item_.selected < 0)
                item_.selected = size;

            if (size < max_entries)
                start_ = 0;
            else if (start_ > item_.selected)
                start_--;
            else if ((static_cast<u32>(item_.selected) == size) && (size > (max_entries - 1)))
                start_ = size - (max_entries - 1);

            key_repeat_timestamp_ = osGetTime() + ((k_down & KEY_UP) ? 500 : 100);
        }
        else if ((k_down & KEY_DOWN) || ((k_held_ & KEY_DOWN) && osGetTime() >= key_repeat_timestamp_)) {
            item_.selected++;
            if (static_cast<u32>(item_.selected) > size)
                item_.selected = 0;

            if ((static_cast<u32>(item_.selected) > (start_ + (max_entries - 1))) && ((start_ + (max_entries - 1)) < size))
                start_++;
            if (item_.selected == 0)
                start_ = 0;

            key_repeat_timestamp_ = osGetTime() + ((k_down & KEY_DOWN) ? 500 : 100);
        }

        if (k_down & KEY_DLEFT) {
            item_.selected = 0;
            start_ = 0;
        }
        else if (k_down & KEY_DRIGHT) {
            item_.selected = item_.entries.size() - 1;
            if ((item_.entries.size() - 1) > max_entries)
                start_ = size - (max_entries - 1);
        }

        if (k_down & KEY_A) {
            std::string filename = EntryNameUtf8();

            if (item_.entries[item_.selected].attributes & FS_ATTRIBUTE_DIRECTORY) {
                if (item_.entries.size() != 0) {
                    /* Anchor the current position before entering, so
                     * Back (B) returns exactly here. */
                    PushNavAnchor();

                    if (R_SUCCEEDED(FS::ChangeDirNext(filename, item_.entries, cfg.filter))) {
                        start_ = 0;
                        // Make a copy before resizing our vector.
                        if ((item_.checked_count > 1) && (item_.checked_copy.empty()))
                            item_.checked_copy = item_.checked;

                        item_.checked.resize(item_.entries.size());
                        item_.selected = 0;

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

                switch (file_type) {
                    case FileTypeImage:
                        if (Textures::LoadImageFile(path, &item_.texture)) {
                            OpenDialog(std::make_unique<ImageViewerScreen>(&item_));
                            return;
                        }
                        break;

                    case FileTypeText:
                        if (Notepad::IsTextFile(filename) && Notepad::Open(path, filename)) {
                            OpenDialog(std::make_unique<TextReaderScreen>(&item_));
                            return;
                        }
                        break;

                    case FileTypeZip:
                        if (R_SUCCEEDED(ArchiveHelper::Extract(path)))
                            RefreshListing();
                        break;

                    case FileType3dsx:
                        HBLoader::Boot(path);
                        break;

                    default:
                        break;
                }
            }
        }
        else if (k_down & KEY_B) {
            /* Settle the child's live position before going up; the config
             * snapshot of this session happens on app exit. */
            SaveBrowserPosition();

            if (R_SUCCEEDED(FS::ChangeDirPrev(item_.entries, cfg.filter))) {
                // Make a copy before resizing our vector.
                if (item_.checked_count > 1)
                    item_.checked_copy = item_.checked;

                item_.checked.resize(item_.entries.size());

                /* Re-anchor to how this directory was left by entering it
                 * (fall back to its top when there's no anchor). */
                bool restored = PeekNavAnchor(cfg.cwd);
                if (restored) {
                    item_.selected = pending_anchor_selected_;
                    if ((item_.selected < 0) ||
                        (item_.selected >= static_cast<int>(item_.entries.size())))
                        item_.selected = 0;
                    SetBrowserStart();
                } else {
                    item_.selected = 0;
                    SetBrowserStart();
                }

                cfg.last_selected = item_.selected;
                cfg.last_scroll = start_;
            }
        }
        else if (k_down & KEY_Y) {
            if ((!item_.checked_cwd.empty()) && (item_.checked_cwd.compare(cfg.cwd) != 0))
                ResetCheckbox(&item_);

            item_.checked_cwd = cfg.cwd;
            item_.checked.at(item_.selected) = !item_.checked.at(item_.selected);
            item_.checked_count = std::count(item_.checked.begin(), item_.checked.end(), true);
        }
        else if (k_down & KEY_SELECT)
            RunNetLoader();
        else if (k_down & KEY_X) {
            PushOptions();
            return;
        }

        HandleActionGrid(k_down);

        /* Mirror the live position into the config cache every control
         * frame; Config::Save only happens on dir changes / app exit. */
        SaveBrowserPosition();
    }

    void FileBrowserScreen::HandleActionGrid(u32 k_down) {
        if (!(k_down & KEY_TOUCH))
            return;

        float button_width = 140;
        float button_height = 30;
        float margin = 10;
        float grid_x = (320 - 2 * button_width - margin) / 2;
        float grid_y = 80;

        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
                float x = grid_x + col * (button_width + margin);
                float y = grid_y + row * (button_height + margin);

                if (Touch::Rect(x, y, x + button_width, y + button_height)) {
                    int index = row * 2 + col;
                    switch (index) {
                        case 0: // New File
                            CreateFile();
                            break;
                        case 1: // New Folder
                            CreateFolder();
                            break;
                        case 2: // Properties
                            touch_bar_.SetSection(TouchBar::SECTION_ACTIONS);
                            OpenDialog(std::make_unique<PropertiesScreen>(this, &item_));
                            break;
                        case 3: // Rename
                            RenameSelected();
                            break;
                    }
                    return;
                }
            }
        }

        // NetLoader button (below the grid, full-grid width)
        float netloader_x = grid_x;
        float netloader_y = grid_y + 2 * (button_height + margin);
        if (Touch::Rect(netloader_x, netloader_y, netloader_x + (2 * button_width + margin),
            netloader_y + button_height)) {
            RunNetLoader();
        }
    }

    void FileBrowserScreen::OnTouch(const touchPosition &touch, bool down) {
        /* Touch bar zones live at the top 20px of the bottom screen. Dialogs
         * forward touches there, so this works whether or not the browser is
         * the current screen. */
        bool in_bar = down? (touch.py < 20) : bar_touching_;

        if (in_bar) {
            touch_bar_.OnTouch(touch, down);
            bar_touching_ = down;
        }
    }

}
