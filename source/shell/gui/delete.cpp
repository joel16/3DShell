#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace Options {
    void Delete(MenuItem *item, int *selection) {
        Result ret = 0;
        Log::Close();

        if ((item->checked_count > 1) && (!item->checked_cwd.compare(cfg.cwd))) {
            for (u32 i = 0; i < item->checked.size(); i++) {
                if (item->checked.at(i)) {
                    if (R_FAILED(ret = FS::Delete(&item->entries[i]))) {
                        FS::GetDirList(cfg.cwd, item->entries);
                        GUI::ResetCheckbox(item);
                        break;
                    }
                }
            }
        }
        else
            ret = FS::Delete(&item->entries[item->selected]);

        if (R_SUCCEEDED(ret)) {
            FS::GetDirList(cfg.cwd, item->entries);
            GUI::ResetCheckbox(item);
        }

        GUI::RecalcStorageSize(item);
        Log::Open();
        *selection = 0;
        item->selected = 0;
    }
}
