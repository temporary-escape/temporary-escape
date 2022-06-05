#pragma once

#include "../Library.hpp"
#include "Vector.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API FloodFill {
public:
    explicit FloodFill(const std::vector<Vector2>& positions,
                       const std::unordered_map<size_t, std::vector<size_t>>& connections,
                       const std::unordered_map<size_t, Vector2>& startingPoints)
        : positions(positions), connections(connections), startingPoints(startingPoints) {
    }

    void operator()(const std::function<void(size_t, size_t)>& yield) const;

private:
    const std::vector<Vector2>& positions;
    // Map of position index -> [position index]
    const std::unordered_map<size_t, std::vector<size_t>>& connections;
    // Map of unique ID -> XY coords
    const std::unordered_map<size_t, Vector2>& startingPoints;
};
} // namespace Engine
