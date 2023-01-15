#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <array>
#include <vector>

namespace Engine {
using VoronoiCell = std::vector<std::array<Vector2, 3>>;
struct VoronoiResult {
    std::vector<VoronoiCell> cells;
};

ENGINE_API VoronoiResult computeVoronoiDiagram(const std::vector<Vector2>& points, const std::vector<Vector2>& clip);
} // namespace Engine
