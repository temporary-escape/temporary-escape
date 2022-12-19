#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <unordered_map>
#include <vector>

namespace Engine {
using DelaunayTriangulationResult = std::unordered_map<size_t, std::vector<size_t>>;

ENGINE_API DelaunayTriangulationResult delaunayTriangulation(const std::vector<Vector2>& points);
} // namespace Engine
