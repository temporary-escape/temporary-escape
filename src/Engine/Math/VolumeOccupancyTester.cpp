#include "VolumeOccupancyTester.hpp"
#include "../Server/Lua.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

void VolumeOccupancyTester::add(const Vector3& pos, float radius) {
    items.emplace_back(Item{pos, radius});
}

bool VolumeOccupancyTester::contactTest(const Vector3& pos, float radius) {
    for (const auto& item : items) {
        if (glm::distance(item.pos, pos) <= item.radius + radius) {
            return true;
        }
    }
    return false;
}
