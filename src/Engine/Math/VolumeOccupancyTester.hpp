#pragma once

#include "Vector.hpp"

namespace Engine {
class ENGINE_API VolumeOccupancyTester {
public:
    void add(const Vector3& pos, float radius);
    bool contactTest(const Vector3& pos, float radius);

private:
    struct Item {
        Vector3 pos;
        float radius{0.0f};
    };

    std::vector<Item> items;
};
} // namespace Engine
