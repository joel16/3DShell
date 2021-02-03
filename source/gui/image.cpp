#include "c2d_helper.h"
#include "colours.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    void DisplayImageViewer(MenuItem *item) {
        C2D_TargetClear(top_screen, BLACK_BG);
        C2D::Image(item->textures[0], ((400 - item->textures[0].subtex->width) / 2), ((240 - item->textures[0].subtex->height) / 2));
    }

    void ControlImageViewer(MenuItem *item, u32 *kDown) {
        if (*kDown & KEY_B) {
            for (u32 i = 0; i < item->textures.size(); i++) {
                delete[] item->textures[i].tex;
                delete[] item->textures[i].subtex;
            }

            item->textures.clear();
            item->state = MENU_STATE_FILEBROWSER;
        }
    }
}
