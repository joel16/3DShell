#ifndef _3D_SHELL_UI_LAYOUT_H
#define _3D_SHELL_UI_LAYOUT_H

#include <vector>

#include "control.h"

namespace UI {

/* Horizontal flow layout: positions controls along one axis, skipping
 * invisible ones. LeftToRight packs from anchor_x growing right;
 * RightToLeft packs so the whole group ENDS at anchor_x (right edge
 * anchor), keeping declaration order. Each item contributes its own
 * intrinsic width; the container provides y, height and the gap. */
class FlowLayout {
public:
    struct Item {
        Control *control = nullptr;
        float width = 0.0f;
    };

    static void Flow(std::vector<Item> &items, float y, float height, float anchor_x, float gap, bool right_align) {
        std::vector<Item> visible;
        float total = 0.0f;

        for (Item &item : items) {
            if (item.control && item.control->Visible()) {
                visible.push_back(item);
                total += item.width;
            }
        }

        if (visible.empty())
            return;

        total += gap * static_cast<float>(visible.size() - 1);

        float x = right_align? (anchor_x - total) : anchor_x;
        for (Item &item : visible) {
            item.control->SetBounds(UI::Rect{x, y, item.width, height});
            x += item.width + gap;
        }
    }
};

}

#endif
