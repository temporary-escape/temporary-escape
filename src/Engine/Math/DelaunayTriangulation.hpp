#pragma once

#include "../Library.hpp"
#include "Vector.hpp"
#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API DelaunayTriangulation {
public:
    DelaunayTriangulation() = default;

    void addPosition(const Vector2& value) {
        vertices.push_back(value);
    }

    bool hasConnections(const size_t index) {
        return connections.find(index) != connections.end();
    }

    const std::vector<size_t>& getConnections(const size_t index) {
        return connections.at(index);
    }

    void calculate();

private:
    std::vector<Vector2> vertices;
    std::unordered_map<size_t, std::vector<size_t>> connections;
};
} // namespace Engine
