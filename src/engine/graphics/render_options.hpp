#pragma once

#include "../math/vector.hpp"

namespace Engine {
struct ENGINE_API RenderOptions {
    Vector2i viewport;
    Vector2i shadowsSize;
};
} // namespace Engine
