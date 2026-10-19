#include <algorithm>
#include <cctype>
#include <codecvt>
#include <cstring>
#include <filesystem>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "log.h"
#include "notepad.h"
#include "osk.h"
#include "touch.h"
#include "virtual_keyboard.h"

namespace Notepad {
    static NotepadData data;
    static u64 cursor_blink = 0;
    static bool cursor_visible = true;
    static u64 saved_flash = 0;
    static bool last_save_ok = false;

    static std::string ToLowerExt(std::string ext) {
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    bool IsTextFile(const std::string &filename) {
        std::string ext = ToLowerExt(std::filesystem::path(filename).extension());
        if (ext == ".txt" || ext == ".log" || ext == ".json" || ext == ".xml" ||
            ext == ".ini" || ext == ".cfg" || ext == ".md" || ext == ".cpp" ||
            ext == ".c" || ext == ".h" || ext == ".hpp" || ext == ".py" ||
            ext == ".js" || ext == ".html" || ext == ".css" || ext == ".csv" ||
            ext == ".yml" || ext == ".yaml" || ext == ".toml" || ext == ".lua" ||
            ext == ".sh" || ext == ".rs" || ext == ".s" || ext == ".asm") {
            return true;
        }
        return false;
    }

    static void SplitLines(const std::string &content) {
        data.lines.clear();
        std::string cur;
        for (size_t i = 0; i < content.size(); i++) {
            char c = content[i];
            if (c == '\r') continue;
            if (c == '\n') {
                data.lines.push_back(cur);
                cur.clear();
                if ((int)data.lines.size() >= NOTEPAD_MAX_LINES) break;
            } else if (c == '\t') {
                cur += "    ";
            } else {
                cur += c;
            }
        }
        data.lines.push_back(cur);
        if (data.lines.empty()) data.lines.push_back("");
        if ((int)data.lines.size() > NOTEPAD_MAX_LINES)
            data.lines.resize(NOTEPAD_MAX_LINES);
    }

    bool Open(const std::string &filepath, const std::string &filename) {
        Close();

        std::u16string path_u16 =
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(filepath.data());

        Handle handle = 0;
        if (R_FAILED(FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path_u16.c_str()), FS_OPEN_READ, 0))) {
            Log::Error("Notepad::Open(%s) failed\n", filepath.c_str());
            return false;
        }

        u64 size = 0;
        FSFILE_GetSize(handle, &size);
        if (size > NOTEPAD_MAX_FILESIZE) size = NOTEPAD_MAX_FILESIZE;

        std::string content;
        content.resize((size_t)size);
        if (size > 0) {
            u32 bytes_read = 0;
            if (R_FAILED(FSFILE_Read(handle, &bytes_read, 0, content.data(), (u32)size))) {
                FSFILE_Close(handle);
                return false;
            }
            content.resize(bytes_read);
        }
        FSFILE_Close(handle);

        data.filepath = filepath;
        data.filename = filename;
        SplitLines(content);
        data.cursor_line = 0;
        data.cursor_col = 0;
        data.scroll_offset = 0;
        data.scroll_x = 0;
        data.mode = NOTEPAD_VIEW;
        data.modified = false;
        data.open = true;
        VirtualKeyboard::Reset();
        return true;
    }

    void Close(void) {
        data.lines.clear();
        data.filename.clear();
        data.filepath.clear();
        data.cursor_line = 0;
        data.cursor_col = 0;
        data.scroll_offset = 0;
        data.scroll_x = 0;
        data.mode = NOTEPAD_VIEW;
        data.modified = false;
        data.open = false;
        saved_flash = 0;
    }

    bool IsOpen(void) { return data.open; }
    bool IsModified(void) { return data.modified; }
    NotepadData &GetData(void) { return data; }

    bool Save(void) {
        if (!data.open) return false;

        std::string out;
        for (size_t i = 0; i < data.lines.size(); i++) {
            out += data.lines[i];
            if (i + 1 < data.lines.size()) out += "\n";
        }

        std::u16string path_u16 =
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(data.filepath.data());

        // Truncate then write. Open WRITE, set size 0, write.
        Handle handle = 0;
        Result ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path_u16.c_str()), FS_OPEN_WRITE, 0);
        if (R_FAILED(ret)) {
            // Try create then open (should already exist, but just in case)
            FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0, out.size());
            ret = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path_u16.c_str()), FS_OPEN_WRITE, 0);
            if (R_FAILED(ret)) {
                saved_flash = osGetTime() + 1500;
                last_save_ok = false;
                return false;
            }
        }

        FSFILE_SetSize(handle, out.size());
        if (!out.empty()) {
            u32 written = 0;
            ret = FSFILE_Write(handle, &written, 0, out.c_str(), (u32)out.size(), FS_WRITE_FLUSH);
            FSFILE_Close(handle);
            if (R_FAILED(ret)) {
                saved_flash = osGetTime() + 1500;
                last_save_ok = false;
                return false;
            }
        } else {
            FSFILE_Close(handle);
        }

        data.modified = false;
        saved_flash = osGetTime() + 1500;
        last_save_ok = true;
        return true;
    }

    void InsertChar(char c) {
        if (!data.open || data.lines.empty()) return;
        if ((int)data.lines.size() > NOTEPAD_MAX_LINES) return;
        std::string &line = data.lines[data.cursor_line];
        if ((int)line.size() > 512) return;
        if (data.cursor_col < 0) data.cursor_col = 0;
        if (data.cursor_col > (int)line.size()) data.cursor_col = (int)line.size();
        line.insert(line.begin() + data.cursor_col, c);
        data.cursor_col++;
        data.modified = true;
    }

    void InsertString(const std::string &s) {
        for (char c : s) {
            if (c == '\n') NewLine();
            else InsertChar(c);
        }
    }

    void Backspace(void) {
        if (!data.open || data.lines.empty()) return;
        std::string &line = data.lines[data.cursor_line];
        if (data.cursor_col > 0 && data.cursor_col <= (int)line.size()) {
            line.erase(line.begin() + (data.cursor_col - 1));
            data.cursor_col--;
            data.modified = true;
        } else if (data.cursor_col == 0 && data.cursor_line > 0) {
            std::string cur = line;
            data.lines.erase(data.lines.begin() + data.cursor_line);
            data.cursor_line--;
            data.cursor_col = (int)data.lines[data.cursor_line].size();
            data.lines[data.cursor_line] += cur;
            if (data.scroll_offset > data.cursor_line) data.scroll_offset = data.cursor_line;
            data.modified = true;
        }
    }

    void NewLine(void) {
        if (!data.open || (int)data.lines.size() >= NOTEPAD_MAX_LINES) return;
        std::string &line = data.lines[data.cursor_line];
        if (data.cursor_col < 0) data.cursor_col = 0;
        if (data.cursor_col > (int)line.size()) data.cursor_col = (int)line.size();
        std::string rest = line.substr(data.cursor_col);
        line = line.substr(0, data.cursor_col);
        data.lines.insert(data.lines.begin() + data.cursor_line + 1, rest);
        data.cursor_line++;
        data.cursor_col = 0;
        data.scroll_x = 0;
        if (data.cursor_line >= data.scroll_offset + NOTEPAD_VISIBLE_LINES)
            data.scroll_offset = data.cursor_line - NOTEPAD_VISIBLE_LINES + 1;
        data.modified = true;
    }

    static void ClampCursor(void) {
        if (data.lines.empty()) return;
        if (data.cursor_line < 0) data.cursor_line = 0;
        if (data.cursor_line >= (int)data.lines.size()) data.cursor_line = (int)data.lines.size() - 1;
        int len = (int)data.lines[data.cursor_line].size();
        if (data.cursor_col < 0) data.cursor_col = 0;
        if (data.cursor_col > len) data.cursor_col = len;
        if (data.cursor_line < data.scroll_offset) data.scroll_offset = data.cursor_line;
        if (data.cursor_line >= data.scroll_offset + NOTEPAD_VISIBLE_LINES)
            data.scroll_offset = data.cursor_line - NOTEPAD_VISIBLE_LINES + 1;
        if (data.cursor_col < data.scroll_x) data.scroll_x = data.cursor_col;
        if (data.cursor_col >= data.scroll_x + NOTEPAD_VISIBLE_COLS)
            data.scroll_x = data.cursor_col - NOTEPAD_VISIBLE_COLS + 1;
        if (data.scroll_x < 0) data.scroll_x = 0;
    }

    void DisplayTop(void) {
        if (!data.open) return;

        std::string title = data.filename;
        if (data.modified) title += "*";
        if (title.length() > 45) title = title.substr(0, 45) + "...";
        C2D::Textf(5, 40, 0.45f, cfg.dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "%s", title.c_str());

        // Content
        float y = 60;
        const float line_h = 14.5f;
        for (int i = 0; i < NOTEPAD_VISIBLE_LINES; i++) {
            int idx = data.scroll_offset + i;
            if (idx >= (int)data.lines.size()) break;
            std::string view = data.lines[idx];
            if ((int)view.length() > data.scroll_x)
                view = view.substr(data.scroll_x, NOTEPAD_VISIBLE_COLS);
            else
                view = "";

            bool is_cursor_line = (idx == data.cursor_line);
            if (is_cursor_line && data.mode == NOTEPAD_EDIT)
                C2D::Rect(3, y - 1, 394, line_h + 2, cfg.dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

            // Cursor blink
            if (is_cursor_line && data.mode == NOTEPAD_EDIT && cursor_visible) {
                int col_vis = data.cursor_col - data.scroll_x;
                if (col_vis < 0) col_vis = 0;
                // Approx char width at 0.42 scale ~ 7px
                float cx = 8 + (col_vis * 6.5f);
                C2D::Rect(cx, y - 1, 2, line_h + 2, cfg.dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR);
            }

            C2D::Text(8, y, 0.42f, cfg.dark_theme ? WHITE : BLACK, view.c_str());
            y += line_h;
        }

        // Status bar bottom of top screen
        const char *mode_str = (data.mode == NOTEPAD_VIEW) ? "View" : "Edit";
        C2D::Textf(5, 222, 0.40f, cfg.dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
            "L%d/%d C%d %s%s", data.cursor_line + 1, (int)data.lines.size(), data.cursor_col + 1, mode_str,
            data.modified ? " *" : "");

        // Transient save feedback
        if (osGetTime() < saved_flash) {
            const char *msg = last_save_ok ? "Saved" : "Save failed";
            u32 colour = last_save_ok ? SUCCESS_COLOUR : ERROR_COLOUR;
            float msg_width = 0.0f;
            C2D::GetTextSize(0.40f, &msg_width, nullptr, msg);
            C2D::Text(395 - msg_width, 222, 0.40f, colour, msg);
        }
    }

    void DisplayBottom(void) {
        if (!data.open) return;

        if (data.mode == NOTEPAD_VIEW) {
            C2D::Text(10, 30, 0.45f, cfg.dark_theme ? WHITE : BLACK, "Text Reader");
            C2D::Text(10, 50, 0.40f, cfg.dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "A:Edit  X:Save");
            C2D::Text(10, 66, 0.40f, cfg.dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "B:Back  Pad:Move");

            // Buttons
            C2D::Rect(10, 100, 90, 30, cfg.dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            C2D::Text(30, 107, 0.42f, cfg.dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "Edit");
            C2D::Rect(115, 100, 90, 30, cfg.dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            C2D::Text(128, 107, 0.42f, cfg.dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "Save");
            C2D::Rect(10, 140, 90, 30, cfg.dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            C2D::Text(32, 147, 0.42f, cfg.dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "Back");
            if (data.modified)
                C2D::Text(115, 147, 0.40f, cfg.dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "* unsaved");
        } else {
            VirtualKeyboard::Display();
        }
    }

    void Control(u32 kDown, u32 kHeld) {
        if (!data.open || data.lines.empty()) return;

        // Blink
        if (osGetTime() >= cursor_blink) {
            cursor_visible = !cursor_visible;
            cursor_blink = osGetTime() + 400;
        }

        if (data.mode == NOTEPAD_VIEW) {
            if ((kDown & KEY_UP) || (kHeld & KEY_UP)) {
                data.cursor_line--;
                ClampCursor();
            } else if ((kDown & KEY_DOWN) || (kHeld & KEY_DOWN)) {
                data.cursor_line++;
                ClampCursor();
            } else if ((kDown & KEY_LEFT) || (kHeld & KEY_LEFT)) {
                data.cursor_col--;
                ClampCursor();
            } else if ((kDown & KEY_RIGHT) || (kHeld & KEY_RIGHT)) {
                data.cursor_col++;
                ClampCursor();
            }

            if (kDown & KEY_A) {
                data.mode = NOTEPAD_EDIT;
                VirtualKeyboard::Reset();
            } else if (kDown & KEY_X) {
                Save();
            } else if (kDown & KEY_B) {
                Close();
                return;
            }

            // Touch buttons in view mode
            if (kDown & KEY_TOUCH) {
                if (Touch::Rect(10, 100, 100, 130)) {
                    data.mode = NOTEPAD_EDIT;
                    VirtualKeyboard::Reset();
                } else if (Touch::Rect(115, 100, 205, 130)) {
                    Save();
                } else if (Touch::Rect(10, 140, 100, 170)) {
                    Close();
                    return;
                }
            }
        } else { // EDIT
            if ((kDown & KEY_UP) || (kHeld & KEY_UP)) {
                data.cursor_line--;
                ClampCursor();
            } else if ((kDown & KEY_DOWN) || (kHeld & KEY_DOWN)) {
                data.cursor_line++;
                ClampCursor();
            } else if ((kDown & KEY_LEFT) || (kHeld & KEY_LEFT)) {
                data.cursor_col--;
                ClampCursor();
            } else if ((kDown & KEY_RIGHT) || (kHeld & KEY_RIGHT)) {
                data.cursor_col++;
                ClampCursor();
            }

            if (kDown & KEY_B) {
                data.mode = NOTEPAD_VIEW; // back to view, keeps changes
            } else if (kDown & KEY_X || kDown & KEY_A) {
                // Physical keyboard has no on-screen input here; X/A = save without exiting
                Save();
            } else if (kDown & KEY_SELECT) {
                // Edit current line with the system keyboard
                std::string cur = data.lines[data.cursor_line];
                std::string res = OSK::GetText(cur, "Edit line");
                if (!res.empty() && res != cur) {
                    data.lines[data.cursor_line] = res;
                    data.cursor_col = (int)res.size();
                    data.modified = true;
                    ClampCursor();
                }
            } else if (kDown & KEY_START) {
                Save();
                data.mode = NOTEPAD_VIEW;
            }

            VirtualKeyboard::Control(kDown);
            ClampCursor();
        }
    }
}
