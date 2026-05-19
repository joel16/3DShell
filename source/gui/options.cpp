#include <codecvt>
#include <locale>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "osk.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static int row = 0, column = 0;
static bool copy = false, move = false, moreOptions = false;

namespace Options {
    static void ResetSelector(void) {
        row = 0;
        column = 0;
    }

    static void HandleMultipleCopy(GuiData& data, Result (*func)()) {
        Result ret = 0;
        std::vector<FS_DirectoryEntry> entries;
        
        if (R_FAILED(ret = FS::GetDirList(data.checkedCwd.data(), entries))) {
            return;
        }
            
        for (u32 i = 0; i < data.checkedCopy.size(); i++) {
            if (data.checkedCopy.at(i)) {
                FS::Copy(&entries[i], data.checkedCwd);
                if (R_FAILED((*func)())) {
                    FS::GetDirList(cfg.cwd, data.entries);
                    GUI::ResetCheckbox(data);
                    break;
                }
            }
        }
        
        FS::GetDirList(cfg.cwd, data.entries);
        GUI::ResetCheckbox(data);
        entries.clear();
    }

    static void CreateFolder(GuiData& data) {
        std::u16string name = OSK::GetText("New Folder", "Enter folder name");
        
        if (R_SUCCEEDED(FS::CreateFolder(name))) {
            FS::GetDirList(cfg.cwd, data.entries);
            GUI::ResetCheckbox(data);
        }
    }

    static void CreateFile(GuiData& data) {
        std::u16string name = OSK::GetText("New File", "Enter file name");
        
        if (R_SUCCEEDED(FS::CreateFile(name))) {
            FS::GetDirList(cfg.cwd, data.entries);
            GUI::ResetCheckbox(data);
        }
    }

    static void Rename(GuiData& data, const std::u16string &filename) {
        std::u16string path = OSK::GetText(Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(filename.c_str())), "Enter new name");
        std::string renamePath = Utils::UTF16ToUTF8(reinterpret_cast<const u16*>(path.c_str()));
        Log::Error("renamePath %s\n", renamePath.c_str());

        if (R_SUCCEEDED(FS::Rename(&data.entries[data.selected], path.c_str()))) {
            FS::GetDirList(cfg.cwd, data.entries);
            Options::ResetSelector();
            moreOptions = false;
            data.state = GUI_STATE_FILEBROWSER;
        }
    }

    static void Copy(GuiData& data) {
        if (!copy) {
            if ((data.checkedCount >= 1) && (data.checkedCwd.compare(cfg.cwd) != 0)) {
                GUI::ResetCheckbox(data);
            }
            if (data.checkedCount <= 1) {
                FS::Copy(&data.entries[data.selected], cfg.cwd);
            }
            
            copy = !copy;
            data.state = GUI_STATE_FILEBROWSER;
        }
        else {
            if ((data.checkedCount > 1) && (data.checkedCwd.compare(cfg.cwd) != 0)) {
                Options::HandleMultipleCopy(data, &FS::Paste);
            }
            else {
                if (R_SUCCEEDED(FS::Paste())) {
                    FS::GetDirList(cfg.cwd, data.entries);
                    GUI::ResetCheckbox(data);
                }
            }
            
            GUI::LoadStorageBar(data);
            copy = !copy;
            data.state = GUI_STATE_FILEBROWSER;
        }
    }

    static void Move(GuiData& data) {
        if (!move) {
            if ((data.checkedCount >= 1) && (data.checkedCwd.compare(cfg.cwd) != 0)) {
                GUI::ResetCheckbox(data);
            }
                
            if (data.checkedCount <= 1) {
                FS::Copy(&data.entries[data.selected], cfg.cwd);
            }
        }
        else {
            if ((data.checkedCount > 1) && (data.checkedCwd.compare(cfg.cwd) != 0)) {
                Options::HandleMultipleCopy(data, &FS::Move);
            }
            else if (R_SUCCEEDED(FS::Move())) {
                FS::GetDirList(cfg.cwd, data.entries);
                GUI::ResetCheckbox(data);
            }
        }
        
        move = !move;
        data.state = GUI_STATE_FILEBROWSER;
    }
}

namespace GUI {
    static float cancelWidth = 0.f, cancelHeight = 0.f;

    void DisplayFileOptions(GuiData& data) {
        GUI::DrawImage(optionsDialog[cfg.theme], 54, 30);
        GUI::DrawText(61, 34, 0.42f, guiTitleColour[cfg.theme], "Actions");
        
        GUI::GetTextDimensions(0.42f, &cancelWidth, &cancelHeight, "CANCEL");
        
        if (row == 0 && column == 0) {
            GUI::DrawRect(56, 69, 103, 36, guiSelectorColour[cfg.theme]);
        }
        else if (row == 1 && column == 0) {
            GUI::DrawRect(160, 69, 103, 36, guiSelectorColour[cfg.theme]);
        }
        else if (row == 0 && column == 1) {
            GUI::DrawRect(56, 105, 103, 36, guiSelectorColour[cfg.theme]);
        }
        else if (row == 1 && column == 1) {
            GUI::DrawRect(160, 105, 103, 36, guiSelectorColour[cfg.theme]);
        }
        else if (row == 0 && column == 2 && !moreOptions) {
            GUI::DrawRect(56, 142, 103, 36, guiSelectorColour[cfg.theme]);
        }
        else if (row == 1 && column == 2 && !moreOptions) {
            GUI::DrawRect(160, 142, 103, 36, guiSelectorColour[cfg.theme]);
        }
        else if (column == 3 && !moreOptions) {
            GUI::DrawRect((256 - cancelWidth) - 5, (221 - cancelHeight) - 5, cancelWidth+ 10, cancelHeight + 10, guiSelectorColour[cfg.theme]);
        }
        else if (column == 2 && moreOptions) {
            GUI::DrawRect((256 - cancelWidth) - 5, (221 - cancelHeight) - 5, cancelWidth + 10, cancelHeight + 10, guiSelectorColour[cfg.theme]);
        }
            
        GUI::DrawText(256 - cancelWidth, 221 - cancelHeight - 3, 0.42f, guiTitleColour[cfg.theme], "CANCEL");
        
        if (!moreOptions) {
            GUI::DrawText(66, 78, 0.42f, guiTextColour[cfg.theme], "Properties");
            GUI::DrawText(66, 114, 0.42f, guiTextColour[cfg.theme], copy? "Paste" : "Copy");
            GUI::DrawText(66, 150, 0.42f, guiTextColour[cfg.theme], "Delete");
            GUI::DrawText(170, 78, 0.42f, guiTextColour[cfg.theme], "Refresh");
            GUI::DrawText(170, 114, 0.42f, guiTextColour[cfg.theme], move? "Paste" : "Move");
            GUI::DrawText(170, 150, 0.42f, guiTextColour[cfg.theme], "More...");
        }
        else {
            GUI::DrawText(66, 78, 0.42f, guiTextColour[cfg.theme], "New folder");
            GUI::DrawText(66, 114, 0.42f, guiTextColour[cfg.theme], "Rename");
            GUI::DrawText(170, 78, 0.42f, guiTextColour[cfg.theme], "New file");
        }
    }

    void ControlFileOptions(GuiData& data, u32& kDown) {
        if (kDown & KEY_RIGHT) {
            row++;
        }
        else if (kDown & KEY_LEFT) {
            row--;
        }
        
        if (kDown & KEY_DDOWN) {
            column++;
        }
        else if (kDown & KEY_DUP) {
            column--;
        }

        if (!moreOptions) {
            Utils::Wrap(row, 0, 1);
            Utils::Wrap(column, 0, 3);
        }
        else {
            Utils::Wrap(column, 0, 2);
            
            if (column == 1) {
                Utils::Wrap(row, 0, 0);
            }
            else {
                Utils::Wrap(row, 0, 1);
            }
        }

        if (kDown & KEY_A) {
            if (row == 0) {
                if (!moreOptions) {
                    if (column == 0) {
                        data.state = GUI_STATE_PROPERTIES;
                    }
                    else if (column == 1) {
                        Options::Copy(data);
                    }
                    else if (column == 2) {
                        data.state = GUI_STATE_DELETE;
                    }
                }
                else {
                    if (column == 0) {
                        Options::CreateFolder(data);
                    }
                    else if (column == 1) {
                        Options::Rename(data, reinterpret_cast<const char16_t *>(data.entries[data.selected].name));
                    }
                }
            }
            else if (row == 1) {
                if (!moreOptions) {
                    if (column == 0) {
                        FS::GetDirList(cfg.cwd, data.entries);
                        Options::ResetSelector();
                        moreOptions = false;
                        data.selected = 0;
                        data.state = GUI_STATE_FILEBROWSER;
                    }
                    else if (column == 1) {
                        Options::Move(data);
                    }
                    else if (column == 2) {
                        Options::ResetSelector();
                        moreOptions = true;
                    }
                }
                else {
                    if (column == 0) {
                        Options::CreateFile(data);
                    }
                }
            }
            if (column == 3) {
                copy = false;
                move = false;
                Options::ResetSelector();
                moreOptions = false;
                data.state = GUI_STATE_FILEBROWSER;
            }
        }
        if (kDown & KEY_B) {
            Options::ResetSelector();

            if (!moreOptions) {
                data.state = GUI_STATE_FILEBROWSER;
            }
            else {
                moreOptions = false;
            }
        }

        if (Touch::Rect(56, 69, 159, 104)) {
            row = 0;
            column = 0;
            
            if (kDown & KEY_TOUCH) {
                if (!moreOptions) {
                    data.state = GUI_STATE_PROPERTIES;
                }
                else {
                    Options::CreateFolder(data);
                }
            }
        }
        else if (Touch::Rect(160, 69, 263, 104)) {
            row = 1;
            column = 0;
            
            if (kDown & KEY_TOUCH) {
                if (!moreOptions) {
                    FS::GetDirList(cfg.cwd, data.entries);
                    Options::ResetSelector();
                    moreOptions = false;
                    data.selected = 0;
                    data.state = GUI_STATE_FILEBROWSER;
                }
                else {
                    Options::CreateFile(data);
                }
            }
        }
        else if (Touch::Rect(56, 105, 159, 141)) {
            row = 0;
            column = 1;
            
            if (kDown & KEY_TOUCH) {
                if (!moreOptions) {
                    Options::Copy(data);
                }
                else {
                    Options::Rename(data, reinterpret_cast<const char16_t *>(data.entries[data.selected].name));
                }
            }
        }
        else if ((Touch::Rect(160, 105, 263, 141)) && (!moreOptions)) {
            row = 1;
            column = 1;
            
            if (kDown & KEY_TOUCH) {
                Options::Move(data);
            }
        }
        else if ((Touch::Rect(56, 142, 159, 178)) && (!moreOptions)) {
            row = 0;
            column = 2;
            
            if (kDown & KEY_TOUCH) {
                data.state = GUI_STATE_DELETE;
            }
        }
        else if ((Touch::Rect(160, 142, 263, 178)) && (!moreOptions)) {
            row = 1;
            column = 2;
            
            if (kDown & KEY_TOUCH) {
                Options::ResetSelector();
                moreOptions = true;
            }
        }
        else if (Touch::Rect((256 - cancelWidth) - 5, (221 - cancelHeight) - 5, ((256 - cancelWidth) - 5) + cancelWidth + 10, 
            ((221 - cancelHeight) - 5) + cancelHeight + 10)) {
            if (moreOptions) {
                column = 2;
            }
            else {
                column = 3;
            }
                
            if (kDown & KEY_TOUCH) {
                Options::ResetSelector();
                moreOptions = false;
                copy = false;
                move = false;
                data.state = GUI_STATE_FILEBROWSER;
            }
        }
    }
}
