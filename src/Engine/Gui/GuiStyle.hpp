#pragma once

#include "../Math/Vector.hpp"

struct nk_style_button;

namespace Engine {
struct GuiStyleColor {
    Color4 normal;
    Color4 hover;
    Color4 active;
};

struct GuiStyleProgress {
    GuiStyleColor border;
    GuiStyleColor bar;
};

struct GuiStyleButton {
    GuiStyleButton(const GuiStyleColor& color, const GuiStyleColor& text);
    ~GuiStyleButton();

    std::unique_ptr<nk_style_button> nk;
};
} // namespace Engine
