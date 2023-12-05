#include "GalaxyDistribution.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static constexpr auto ARMS_COUNT = 5;
static constexpr auto ARM_ANGLE = static_cast<float>(2.0 * glm::pi<double>() / double(ARMS_COUNT));
static constexpr auto ARM_ANGLE_HALF = ARM_ANGLE / 2.0f - ARM_ANGLE * 0.05f;

GalaxyDistribution::GalaxyDistribution(const float maxWidth, const float division, const float offset) :
    maxWidth{maxWidth},
    division{division},
    distArmIndex{0, ARMS_COUNT},
    distArmDistance{0.01f, 1.0f},
    distArmOffset{-ARM_ANGLE_HALF, ARM_ANGLE_HALF},
    distGridOffset{-offset, offset} {
}

std::optional<Vector2> GalaxyDistribution::operator()(std::mt19937_64& rng) {
    auto count = 100;
    while (count-- > 0) {
        const auto dist = distArmDistance(rng);
        auto angleOffset = (distArmOffset(rng) * (1.0f / dist));
        if (angleOffset > 0.0f) {
            angleOffset = std::pow(angleOffset, 2.0f);
        } else {
            angleOffset = -std::pow(-angleOffset, 2.0f);
        }
        auto angle = distArmIndex(rng) * ARM_ANGLE + angleOffset;

        angle = angle + dist * 2.0f;

        auto vec = Vector2{dist * maxWidth, 0.0f};

        const auto ca = std::cos(angle);
        const auto sa = std::sin(angle);
        vec = Vector2{ca * vec.x - sa * vec.y, sa * vec.x + ca * vec.y};

        const auto fixed = Vector2i{vec / division};

        const auto test = *reinterpret_cast<const uint64_t*>(&fixed.x);
        if (positions.find(test) == positions.end()) {
            positions.insert(test);

            const auto pos = Vector2{fixed} * division;
            return Vector2{pos.x + distGridOffset(rng), pos.y + distGridOffset(rng)};
        }
    }

    return {};
}
