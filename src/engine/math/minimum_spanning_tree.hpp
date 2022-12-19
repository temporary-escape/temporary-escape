#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <unordered_map>
#include <vector>

namespace Engine {
using MinimumSpanningTreeResult = std::unordered_map<size_t, std::vector<size_t>>;

ENGINE_API MinimumSpanningTreeResult minimumSpanningTree(const std::vector<Vector2>& positions);
} // namespace Engine
