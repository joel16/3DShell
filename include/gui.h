#ifndef _3D_SHELL_GUI_H
#define _3D_SHELL_GUI_H

#include <3ds.h>
#include <citro2d.h>
#include <vector>

enum MENU_STATES {
    MENU_STATE_FILEBROWSER,
    MENU_STATE_OPTIONS,
    MENU_STATE_DELETE,
    MENU_STATE_PROPERTIES,
    MENU_STATE_SETTINGS,
    MENU_STATE_IMAGEVIEWER,
    MENU_STATE_ARCHIVEEXTRACT,
    MENU_STATE_TEXTREADER,
    MENU_STATE_UPDATE
};

typedef struct {
    MENU_STATES state = MENU_STATE_FILEBROWSER;
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

namespace GUI {
    void ResetCheckbox(MenuItem *item);
    void RecalcStorageSize(MenuItem *item);
    void ProgressBar(const std::string &title, std::string message, u64 offset, u64 size);
    void DownloadProgressBar(void *args);
    Result Loop(void);

    // Windows
    void DisplayFileBrowser(MenuItem *item);
    void ControlFileBrowser(MenuItem *item, u32 *kDown, u32 *kHeld);
    void DisplayFileOptions(MenuItem *item);
    void ControlFileOptions(MenuItem *item, u32 *kDown);
    void DisplayProperties(MenuItem *item);
    void ControlProperties(MenuItem *item, u32 *kDown);
    void DisplaySettings(MenuItem *item);
    void ControlSettings(MenuItem *item, u32 *kDown);
    void DisplayImageViewerTop(MenuItem *item);
    void DisplayImageViewerBottom(MenuItem *item);
    void ControlImageViewer(MenuItem *item, u32 *kDown, u32 *kHeld, u64 *delta_time);
    void DisplayDeleteOptions(MenuItem *item);
    void ControlDeleteOptions(MenuItem *item, u32 *kDown);
    void DisplayUpdateOptions(bool *connection_status, bool *available, const std::string &tag);
    void ControlUpdateOptions(MenuItem *item, u32 *kDown, bool *state, bool *connection_status, bool *available, const std::string &tag);
}

#endif
