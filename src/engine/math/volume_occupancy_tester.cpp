#include "volume_occupancy_tester.hpp"
#include "../server/lua.hpp"
#include "../utils/exceptions.hpp"
#include <sol/sol.hpp>

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

void VolumeOccupancyTester::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls =
        m.new_usertype<VolumeOccupancyTester>("VolumeOccupancyTester", sol::constructors<VolumeOccupancyTester()>{});
    cls["add"] = &VolumeOccupancyTester::add;
    cls["contact_test"] = &VolumeOccupancyTester::contactTest;
}
