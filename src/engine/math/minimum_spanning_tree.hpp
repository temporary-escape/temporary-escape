#pragma once

#include "vector.hpp"
#include <unordered_map>
#include <vector>

namespace Engine {
class MinimumSpanningTree {
public:
    using Result = std::unordered_map<size_t, std::vector<size_t>>;

    explicit MinimumSpanningTree(const std::vector<Vector2>& positions);
    Result solve();

private:
    const std::vector<Vector2>& positions;
};
} // namespace Engine
