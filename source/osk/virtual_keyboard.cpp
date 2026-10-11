#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "notepad.h"
#include "touch.h"
#include "virtual_keyboard.h"

namespace VirtualKeyboard {
    static Layout current = LAYOUT_LOWER;

    static const char *rows_lower[3] = { "qwertyuiop", "asdfghjkl", "zxcvbnm" };
    static const char *rows_upper[3] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };
    static const char *rows_num[3] = { "1234567890", "-/:;()&@\"", ".,?!'" };
    static const char *rows_sym[3] = { "[]{}#%^*+=", "_\\|~<>$`", ".,?!'" };

    void Reset(void) { current = LAYOUT_LOWER; }
    Layout GetLayout(void) { return current; }
    void SetLayout(Layout l) { current = l; }

    static const char **ActiveRows(void) {
        switch (current) {
            case LAYOUT_UPPER: return (const char **)rows_upper;
            case LAYOUT_NUMERIC: return (const char **)rows_num;
            case LAYOUT_SYMBOLS: return (const char **)rows_sym;
            default: return (const char **)rows_lower;
        }
    }

    static void DrawKey(float x, float y, float w, float h, const std::string &label, bool highlight) {
        C2D::Rect(x, y, w, h, highlight ? (cfg.dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR)
                                        : (cfg.dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT));
        float tw = 0, th = 0;
        C2D::GetTextSize(0.42f, &tw, &th, label);
        C2D::Text(x + (w - tw) / 2, y + (h - th) / 2, 0.42f, cfg.dark_theme ? WHITE : BLACK, label);
    }

    void Display(void) {
        const char **rows = ActiveRows();

        const float key_w = 29.0f, key_h = 30.0f, gap = 2.0f;
        float start_y = 45.0f;

        // 3 filas de letras/simbolos
        for (int r = 0; r < 3; r++) {
            std::string row = rows[r];
            float total_w = row.size() * (key_w + gap) - gap;
            float start_x = (320.0f - total_w) / 2.0f;
            for (size_t i = 0; i < row.size(); i++) {
                float x = start_x + i * (key_w + gap);
                float y = start_y + r * (key_h + gap);
                char buf[2] = { row[i], 0 };
                DrawKey(x, y, key_w, key_h, buf, false);
            }
        }

        // Fila inferior: SHIFT | 123 | ESPACIO | BKSP | ENTER
        float y = start_y + 3 * (key_h + gap) + 4;
        DrawKey(8, y, 44, key_h, current == LAYOUT_LOWER ? "SHIFT" : "shift", false);
        const char *mid = (current == LAYOUT_NUMERIC || current == LAYOUT_SYMBOLS) ? "ABC" : "123";
        DrawKey(54, y, 44, key_h, mid, false);
        DrawKey(100, y, 108, key_h, "espacio", false);
        DrawKey(210, y, 50, key_h, "BKSP", false);
        DrawKey(262, y, 50, key_h, "ENTER", false);

        // Hint superior
        const char *lname = "abc";
        if (current == LAYOUT_UPPER) lname = "ABC";
        else if (current == LAYOUT_NUMERIC) lname = "123";
        else if (current == LAYOUT_SYMBOLS) lname = "#+=";
        C2D::Textf(8, 26, 0.38f, cfg.dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
            "Teclado [%s]  SELECT:linea  X/A:guardar  B:vista", lname);
    }

    static bool InRect(float x, float y, float w, float h) {
        return Touch::Rect((u16)x, (u16)y, (u16)(x + w), (u16)(y + h));
    }

    void Control(u32 kDown) {
        if (!(kDown & KEY_TOUCH)) return;

        const char **rows = ActiveRows();
        const float key_w = 29.0f, key_h = 30.0f, gap = 2.0f;
        float start_y = 45.0f;

        for (int r = 0; r < 3; r++) {
            std::string row = rows[r];
            float total_w = row.size() * (key_w + gap) - gap;
            float start_x = (320.0f - total_w) / 2.0f;
            for (size_t i = 0; i < row.size(); i++) {
                float x = start_x + i * (key_w + gap);
                float y = start_y + r * (key_h + gap);
                if (InRect(x, y, key_w, key_h)) {
                    Notepad::InsertChar(row[i]);
                    return;
                }
            }
        }

        float y = start_y + 3 * (key_h + gap) + 4;
        if (InRect(8, y, 44, key_h)) {
            // Shift toggle lower<->upper, numeric/symbols -> lower
            if (current == LAYOUT_LOWER) current = LAYOUT_UPPER;
            else if (current == LAYOUT_UPPER) current = LAYOUT_LOWER;
            else current = LAYOUT_LOWER;
        } else if (InRect(54, y, 44, key_h)) {
            if (current == LAYOUT_NUMERIC) current = LAYOUT_LOWER;
            else if (current == LAYOUT_SYMBOLS) current = LAYOUT_LOWER;
            else current = LAYOUT_NUMERIC;
            // second press on 123 goes to symbols? handle via long logic:
            // if already numeric, alternate is done on next tap through ABC? keep simple:
        } else if (InRect(100, y, 108, key_h)) {
            Notepad::InsertChar(' ');
        } else if (InRect(210, y, 50, key_h)) {
            Notepad::Backspace();
        } else if (InRect(262, y, 50, key_h)) {
            Notepad::NewLine();
        }
    }
}
