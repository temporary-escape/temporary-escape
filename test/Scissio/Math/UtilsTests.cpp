#include "../Common.hpp"
#include <Math/Utils.hpp>

#define TAG "[Math]"

TEST_CASE("Ray box intersection", TAG) {
    Vector3 min{-0.5f};
    Vector3 max{0.5f};
    Vector3 from{-2.0f, 0.0f, 0.0f};
    Vector3 to{2.0f, 0.0f, 0.0f};

    auto res = intersectBox(min, max, from, to);
    REQUIRE(res.has_value());
    REQUIRE(res.value() == Vector3{-0.5f, 0.0f, 0.0f});

    from = Vector3{2.0f, 0.0f, 0.0f};
    to = Vector3{-2.0f, 0.0f, 0.0f};

    res = intersectBox(min, max, from, to);
    REQUIRE(res.has_value());
    REQUIRE(res.value() == Vector3{0.5f, 0.0f, 0.0f});

    from = Vector3{2.0f, 2.0f, 0.0f};
    to = Vector3{-2.0f, -2.0f, 0.0f};

    res = intersectBox(min, max, from, to);
    REQUIRE(res.has_value());
    REQUIRE(res.value().x == Approx(0.5f));
    REQUIRE(res.value().y == Approx(0.5f));
    REQUIRE(res.value().z == Approx(0.0f));
}
