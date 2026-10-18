#ifndef _3D_SHELL_UI_LABEL_H
#define _3D_SHELL_UI_LABEL_H

#include <string>

#include "../control.h"
#include "../c2d_helper.h"

namespace UI {

class Label : public Control {
public:
    Label(const std::string &text, float x, float y, float size, u32 colour)
        : text_(text), size_(size), colour_(colour) {
        bounds_ = Rect{x, y, 0.0f, 0.0f};
    }

    void Draw() override {
        C2D::Text(bounds_.x, bounds_.y, size_, colour_, text_);
    }

    void SetText(const std::string &text) { text_ = text; }
    const std::string &Text() const { return text_; }

    void SetColour(u32 colour) { colour_ = colour; }

private:
    std::string text_;
    float size_;
    u32 colour_;
};

}

#endif
