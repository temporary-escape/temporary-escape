#include "../../Common.hpp"
#include <Engine/Utils/Chrono.hpp>

using namespace Engine;

TEST_CASE("Parse and format time point", "[chrono]") {
    const auto str = "2024-01-27T17:28:05.123Z";
    const auto tp = isoToTimePoint(str);

    const auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    REQUIRE(epoch.count() == 1706376485123);

    const auto test = timePointToIso(tp);
    REQUIRE(test == str);
}
