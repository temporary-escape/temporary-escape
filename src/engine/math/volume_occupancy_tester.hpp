#pragma once

#include "vector.hpp"

namespace Engine {
class ENGINE_API VolumeOccupancyTester {
public:
    void add(const Vector3& pos, float radius);
    bool contactTest(const Vector3& pos, float radius);

    static void bind(Lua& lua);

private:
    struct Item {
        Vector3 pos;
        float radius{0.0f};
    };

    std::vector<Item> items;
};
} // namespace Engine
