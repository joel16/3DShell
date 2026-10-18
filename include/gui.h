#ifndef _3D_SHELL_GUI_H
#define _3D_SHELL_GUI_H

#include <3ds.h>
#include <citro2d.h>
#include <string>
#include <vector>

typedef struct {
    int selected = 0;
    std::vector<FS_DirectoryEntry> entries;
    std::vector<bool> checked;
    std::vector<bool> checked_copy;
    std::string checked_cwd;
    int checked_count = 0;
    u64 used_storage = 0;
    u64 total_storage = 0;
    C2D_Image texture;
} MenuItem;

namespace Options {
    void Delete(MenuItem *item, int *selection);
}

namespace GUI {
    void ResetCheckbox(MenuItem *item);
    void RecalcStorageSize(MenuItem *item);
    void ProgressBar(const std::string &title, std::string message, u64 offset, u64 size);
    void DownloadProgressBar(void *args);
    Result Loop(void);

    void DisplayProperties(MenuItem *item);
    void DownloadHelper(const std::string &tag);
    bool UpdateDone(void);
}

#endif
