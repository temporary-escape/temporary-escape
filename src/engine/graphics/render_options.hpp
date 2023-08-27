#pragma once

#include "../math/vector.hpp"

namespace Engine {
struct ENGINE_API RenderOptions {
    Vector2i viewport;
    int shadowsSize;
    int shadowsLevel;
    int ssao{0};
    bool fxaa{true};
    bool bloom{false};
    bool enableSrc{false};
};
} // namespace Engine
