#pragma once

#include "../Math/Vector.hpp"

struct nk_color;
struct nk_style_button;
struct nk_context;

namespace Engine {
struct GuiColor {
    uint8_t r, g, b, a;

    const nk_color& nk() const;
};

extern ENGINE_API GuiColor toGuiColor(const Color4& color);

struct ENGINE_API GuiStyleColor {
    GuiColor normal;
    GuiColor hover;
    GuiColor active;
};

struct ENGINE_API GuiStyleProgress {
    GuiStyleColor border;
    GuiStyleColor bar;
    GuiStyleColor background;
};

struct ENGINE_API GuiStyleButton {
    GuiStyleColor text;
    GuiStyleColor border;
    GuiStyleColor color;

    void apply(nk_context* nk, nk_style_button& style) const;
};

struct ENGINE_API GuiStyleWindow {
    GuiColor header;
    GuiColor border;
    GuiColor title;
    GuiColor background;
};

struct ENGINE_API GuiStyleGroup {
    GuiStyleColor border;
    GuiStyleColor background;
};

extern ENGINE_API GuiStyleButton guiStyleButtonBlue;
extern ENGINE_API GuiStyleButton guiStyleButtonBlueOutline;
extern ENGINE_API GuiStyleButton guiStyleButtonGreen;
extern ENGINE_API GuiStyleButton guiStyleButtonGreenOutline;
extern ENGINE_API GuiStyleButton guiStyleButtonYellow;
extern ENGINE_API GuiStyleButton guiStyleButtonYellowOutline;
extern ENGINE_API GuiStyleButton guiStyleButtonRed;
extern ENGINE_API GuiStyleButton guiStyleButtonRedOutline;
extern ENGINE_API GuiStyleButton guiStyleButtonPurple;
extern ENGINE_API GuiStyleButton guiStyleButtonPurpleOutline;
extern ENGINE_API GuiStyleButton guiStyleButtonGray;
extern ENGINE_API GuiStyleButton guiStyleButtonGrayOutline;

extern ENGINE_API GuiStyleWindow guiStyleWindowDefault;
extern ENGINE_API GuiStyleWindow guiStyleWindowYellow;
extern ENGINE_API GuiStyleWindow guiStyleWindowGreen;
extern ENGINE_API GuiStyleWindow guiStyleWindowRed;
extern ENGINE_API GuiStyleWindow guiStyleWindowNone;

extern ENGINE_API GuiStyleProgress guiStyleProgressYellow;
extern ENGINE_API GuiStyleProgress guiStyleProgressGreen;
extern ENGINE_API GuiStyleProgress guiStyleProgressBlue;
extern ENGINE_API GuiStyleProgress guiStyleProgressPurple;

extern ENGINE_API GuiStyleGroup guiStyleGroupDefault;
extern ENGINE_API GuiStyleGroup guiStyleGroupSelectable;
extern ENGINE_API GuiStyleGroup guiStyleGroupNone;
} // namespace Engine
