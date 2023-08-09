#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API FloodFill {
public:
    struct Result {
        size_t index{0};
        size_t point{0};

        static void bind(Lua& lua);
    };

    FloodFill() = default;

    void addPosition(const Vector2& pos, const std::vector<size_t>& conns) {
        connections.insert(std::make_pair(positions.size(), conns));
        positions.push_back(pos);
    }

    void addStartPoint(const Vector2& pos, const size_t id) {
        startingPoints.insert(std::make_pair(id, pos));
    }

    void calculate();

    size_t size() const {
        return results.size();
    }

    const Result& get(const size_t index) {
        return results.at(index);
    }

    static void bind(Lua& lua);

private:
    std::vector<Vector2> positions;
    std::unordered_map<size_t, std::vector<size_t>> connections;
    std::unordered_map<size_t, Vector2> startingPoints;
    std::vector<Result> results;
};
} // namespace Engine
