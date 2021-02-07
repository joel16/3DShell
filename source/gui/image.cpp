#include "c2d_helper.h"
#include "colours.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    void DisplayImageViewer(MenuItem *item) {
        C2D_TargetClear(top_screen, BLACK_BG);
        C2D::Image(item->texture, ((400 - item->texture.subtex->width) / 2), ((240 - item->texture.subtex->height) / 2));
    }

    void ControlImageViewer(MenuItem *item, u32 *kDown) {
        if (*kDown & KEY_B) {
            delete[] item->texture.tex;
            delete[] item->texture.subtex;
            item->state = MENU_STATE_FILEBROWSER;
        }
    }
}
