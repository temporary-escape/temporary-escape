#pragma once

#include "vector.hpp"
#include <random>
#include <unordered_set>

namespace Engine {
class GalaxyDistribution {
public:
    explicit GalaxyDistribution(const float maxWidth) :
        maxWidth{maxWidth * 3.9f},
        distArmIndex{0, ARMS_COUNT},
        distArmDistance{0.01f, 1.0f},
        distArmOffset{-ARM_ANGLE_HALF, ARM_ANGLE_HALF},
        distGridOffset{-0.4f, 0.4f} {
    }

    [[nodiscard]] Vector2 operator()(std::mt19937_64& rng);

private:
    static constexpr auto ARMS_COUNT = 5;
    static constexpr auto ARM_ANGLE = static_cast<float>(2.0 * glm::pi<double>() / double(ARMS_COUNT));
    static constexpr auto ARM_ANGLE_HALF = ARM_ANGLE / 2.0f - ARM_ANGLE * 0.05f;

    float maxWidth;

    std::unordered_set<uint64_t> positions;
    std::uniform_int_distribution<int> distArmIndex;
    std::uniform_real_distribution<float> distArmDistance;
    std::uniform_real_distribution<float> distArmOffset;
    std::uniform_real_distribution<float> distGridOffset;
};
} // namespace Engine
