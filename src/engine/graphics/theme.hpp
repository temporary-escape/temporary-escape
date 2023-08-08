#pragma once

#include "../math/vector.hpp"

namespace Engine {
struct ENGINE_API Theme {
    static inline const auto backgroundTransparent = Color4(0.02f, 0.02f, 0.02f, 1.0f);
    static inline const auto text = Color4(0.9f, 0.9f, 0.9f, 1.0f);
    static inline const auto primary = Color4(0.99f, 0.86f, 0.05f, 1.0f);    // #fcdb0c
    static inline const auto secondary = Color4(0.024f, 0.81f, 0.69f, 1.0f); // #06d1b2
    static inline const auto ternary = Color4(0.91f, 0.0f, 0.05f, 1.0f);     // #e8000d
};
} // namespace Engine
