#ifndef _3D_SHELL_FILE_BROWSER_SCREEN_H
#define _3D_SHELL_FILE_BROWSER_SCREEN_H

#include "gui.h"

#include "controls/scroll_bar.h"
#include "fs.h"
#include "screen.h"
#include "screen_manager.h"

namespace GUI {

class FileBrowserScreen : public UI::Screen {
public:
    FileBrowserScreen();
    ~FileBrowserScreen() override;

    Result Init();

    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    typedef struct BrowserAnchor {
        std::string cwd;
        int selected;
        int scroll;
    } BrowserAnchor;

    static std::size_t u16len(const u16 *src);
    std::string SafeUtf16ToUtf8(const char16_t *src, std::size_t len);
    std::u16string ToUtf16Path(const std::string &path);
    std::string FitText(const std::string &text, float size, float max_width);
    std::string EntryNameUtf8();

    void PushNavAnchor();
    bool PeekNavAnchor(const std::string &cwd);
    void SetBrowserStart();
    void RestoreLastBrowserPosition();
    void SaveBrowserPosition();
    void ControlEntryCache();
    void ControlSmdhPanel();
    void DisplaySmdhPanel();
    void DisplayActionGrid();
    void HandleActionGrid(u32 k_down);
    void HandleTouchBar(u32 k_down);
    void RunNetLoader();
    void CreateFolder();
    void CreateFile();
    void RenameSelected();
    void RefreshListing();
    void PushOptions();
    void PushSettings();
    void OpenDialog(std::unique_ptr<UI::Screen> screen);

    MenuItem item_;
    UI::ScrollBar scroll_bar_;

    int start_ = 0;
    u64 key_repeat_timestamp_ = 0;
    u64 delta_time_ = 0;
    u32 k_held_ = 0;
    bool dialog_open_ = false;
    int pushed_filter_ = -1;
    TouchBarSection touch_section_ = TOUCH_BAR_BROWSER;

    u32 cache_epoch_ = 0;
    std::size_t cache_count_ = 0;
    std::vector<std::string> cache_names_;
    std::vector<FileType> cache_types_;

    std::vector<BrowserAnchor> nav_anchors_;
    int pending_anchor_selected_ = 0;
    int pending_anchor_scroll_ = -1;

    std::u16string smdh_cwd_u16_;
};

}

#endif
