#include "../common.hpp"
#include <engine/math/utils.hpp>
#include <engine/utils/log.hpp>
#include <glm/gtx/vector_angle.hpp>

#define TAG "[Math]"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

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

TEST_CASE("Gift wrap", TAG) {
    std::vector<Vector2> positions = {
        {5.0f, 5.0f}, {10.0f, 5.0f}, {5.0f, 0.0f}, {3.0f, 10.0f}, {7.0f, 10.0f}, {0.0f, 5.0f}, {0.0f, 0.0f},
    };

    std::vector<size_t> connections = {
        1, 6, 3, 4, 5, 2,
    };

    const auto findLargestX = [&]() {
        size_t idx = 0;
        float current = positions.at(connections.at(0)).x;
        for (size_t i = 1; i < connections.size(); i++) {
            const auto& test = positions.at(connections.at(i));
            if (test.x > current) {
                current = test.x;
                idx = i;
            }
        }
        return idx;
    };

    auto idx = findLargestX();

    auto conns = connections;
    decltype(connections) copy;
    conns.erase(conns.begin() + idx);
    copy.push_back(idx);

    Vector2 forward{1.0f, 0.0f};

    auto center = positions.at(0);

    while (!conns.empty()) {
        for (size_t i = 0; i < conns.size(); i++) {
            const auto& pos = positions.at(conns.at(i));
            const auto dir = glm::normalize(pos - center);

            auto test = glm::angle(forward, dir);
            auto testOriented = glm::orientedAngle(forward, dir);

            if (testOriented < 0.0f) {
                testOriented += glm::radians(360.0f);
            }

            logger.debug("Pos: {} dir: {} test: {} oriented: {}", pos, dir, glm::degrees(test),
                         glm::degrees(testOriented));
        }

        break;
    }
}
