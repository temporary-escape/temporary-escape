#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <vector>

namespace Engine {
ENGINE_API std::vector<Vector2> computeConvexHull(std::vector<Vector2> points);
} // namespace Engine
