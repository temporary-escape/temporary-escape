#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace Engine {

ENGINE_API void floodFill(const std::vector<Vector2>& positions,
                          const std::unordered_map<size_t, std::vector<size_t>>& connections,
                          const std::unordered_map<size_t, Vector2>& startingPoints,
                          const std::function<void(size_t, size_t)>& yield);
} // namespace Engine
