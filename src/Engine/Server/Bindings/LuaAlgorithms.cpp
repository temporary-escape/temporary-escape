#include "../../Math/DelaunayTriangulation.hpp"
#include "../../Math/FloodFillPoints.hpp"
#include "../../Math/GalaxyDistribution.hpp"
#include "../../Math/MinimumSpanningTree.hpp"
#include "../../Math/VolumeOccupancyTester.hpp"
#include "Bindings.hpp"

using namespace Engine;

static void bindMinimumSpanningTree(sol::table& m) {
    auto cls = m.new_usertype<MinimumSpanningTree>("MinimumSpanningTree", sol::constructors<MinimumSpanningTree()>{});
    cls["add_position"] = &MinimumSpanningTree::addPosition;
    cls["calculate"] = &MinimumSpanningTree::calculate;
    cls["has_connections"] = &MinimumSpanningTree::hasConnections;
    cls["get_connections"] = [](MinimumSpanningTree& self, const size_t index) {
        return sol::as_table(self.getConnections(index));
    };
}

LUA_BINDINGS(bindMinimumSpanningTree);

static void bindVolumeOccupancyTester(sol::table& m) {
    auto cls =
        m.new_usertype<VolumeOccupancyTester>("VolumeOccupancyTester", sol::constructors<VolumeOccupancyTester()>{});
    cls["add"] = &VolumeOccupancyTester::add;
    cls["contact_test"] = &VolumeOccupancyTester::contactTest;
}

LUA_BINDINGS(bindVolumeOccupancyTester);

static void bindGalaxyDistribution(sol::table& m) {
    auto cls = m.new_usertype<GalaxyDistribution>("GalaxyDistribution",
                                                  sol::constructors<GalaxyDistribution(float, float, float)>{});
    cls["get"] = &GalaxyDistribution::operator();
}

LUA_BINDINGS(bindGalaxyDistribution);

static void bindFloodFill(sol::table& m) {
    {
        auto cls =
            m.new_usertype<FloodFillPoints::Result>("FloodFillResult", sol::constructors<FloodFillPoints::Result()>{});
        cls["index"] = &FloodFillPoints::Result::index;
        cls["point"] = &FloodFillPoints::Result::point;
    }

    { // FloodFill
        auto cls = m.new_usertype<FloodFillPoints>("FloodFill", sol::constructors<FloodFillPoints()>{});
        cls["calculate"] = &FloodFillPoints::calculate;
        cls["add_start_point"] = &FloodFillPoints::addStartPoint;
        cls["size"] = &FloodFillPoints::size;
        cls["get"] = &FloodFillPoints::get;
        cls["add_position"] =
            [](FloodFillPoints& self, const Vector2& pos, sol::as_table_t<std::vector<size_t>> conns) {
                self.addPosition(pos, conns.value());
            };
    }
}

LUA_BINDINGS(bindFloodFill);

static void bindDelaunayTriangulation(sol::table& m) {
    auto cls =
        m.new_usertype<DelaunayTriangulation>("DelaunayTriangulation", sol::constructors<DelaunayTriangulation()>{});
    cls["add_position"] = &DelaunayTriangulation::addPosition;
    cls["calculate"] = &DelaunayTriangulation::calculate;
    cls["has_connections"] = &DelaunayTriangulation::hasConnections;
    cls["get_connections"] = [](DelaunayTriangulation& self, const size_t index) {
        return sol::as_table(self.getConnections(index));
    };
}

LUA_BINDINGS(bindDelaunayTriangulation);
