#pragma once

#include "../Math/Vector.hpp"

namespace Engine {
struct GuiColors {
    static inline const auto backgroundTransparent = Color4(0.05f, 0.05f, 0.05f, 0.4f);
    static inline const auto text = Color4(0.9f, 0.9f, 0.9f, 1.0f);
    static inline const auto primary = Color4(0.99f, 0.86f, 0.05f, 1.0f);
    static inline const auto secondary = Color4(0.024f, 0.81f, 0.69f, 1.0f);
    static inline const auto ternary = Color4(0.91f, 0.0f, 0.05f, 1.0f);
};
} // namespace Engine
