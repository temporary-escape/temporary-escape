#pragma once

#include "../Library.hpp"
#include "Vector.hpp"
#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API MinimumSpanningTree {
public:
    MinimumSpanningTree() = default;

    void addPosition(const Vector2& value) {
        positions.push_back(value);
    }

    bool hasConnections(const size_t index) {
        return results.find(index) != results.end();
    }

    const std::vector<size_t>& getConnections(const size_t index) {
        return results.at(index);
    }

    void calculate();

    static void bind(Lua& lua);

private:
    std::vector<Vector2> positions;
    std::unordered_map<size_t, std::vector<size_t>> results;
};
} // namespace Engine
