#include "GalaxyDistribution.hpp"

using namespace Engine;

Vector2 GalaxyDistribution::operator()(std::mt19937_64& rng) {
    Vector2i fixed;

    auto count = 5;
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

        fixed = Vector2i{vec};

        const auto test = *reinterpret_cast<uint64_t*>(&fixed.x);
        if (positions.find(test) == positions.end()) {
            positions.insert(test);
            break;
        }
    }

    Vector2 pos{fixed};
    pos = Vector2{pos.x + distGridOffset(rng), pos.y + distGridOffset(rng)};

    return pos;
}
