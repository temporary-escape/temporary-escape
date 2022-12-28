#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <array>
#include <vector>

namespace Engine {
using VoronoiCell = std::vector<std::array<Vector2, 3>>;

ENGINE_API std::vector<VoronoiCell> computeVoronoiDiagram(const std::vector<Vector2>& points,
                                                          const std::vector<Vector2>& clip = {});
} // namespace Engine