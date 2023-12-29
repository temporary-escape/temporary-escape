#pragma once

#include "../Math/Vector.hpp"

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
} // namespace Engine
