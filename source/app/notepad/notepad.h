#ifndef _3D_SHELL_NOTEPAD_H
#define _3D_SHELL_NOTEPAD_H

#include <3ds.h>
#include <string>
#include <vector>

#define NOTEPAD_MAX_LINES 1000
#define NOTEPAD_MAX_FILESIZE (256 * 1024)
#define NOTEPAD_VISIBLE_LINES 11
#define NOTEPAD_VISIBLE_COLS 50

typedef enum {
    NOTEPAD_VIEW = 0,
    NOTEPAD_EDIT = 1
} NotepadMode;

typedef struct {
    std::string filename;
    std::string filepath;
    std::vector<std::string> lines;
    int cursor_line = 0;
    int cursor_col = 0;
    int scroll_offset = 0;
    int scroll_x = 0;
    NotepadMode mode = NOTEPAD_VIEW;
    bool modified = false;
    bool open = false;
} NotepadData;

namespace Notepad {
    bool IsTextFile(const std::string &filename);
    bool Open(const std::string &filepath, const std::string &filename);
    void Close(void);
    bool IsOpen(void);
    bool IsModified(void);
    bool Save(void);

    void InsertChar(char c);
    void InsertString(const std::string &s);
    void Backspace(void);
    void NewLine(void);

    NotepadData &GetData(void);

    void DisplayTop(void);
    void DisplayBottom(void);
    void Control(u32 kDown, u32 kHeld);
}

#endif
