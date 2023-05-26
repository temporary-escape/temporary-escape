#pragma once

#include "../library.hpp"
#include "vector.hpp"
#include <random>
#include <unordered_set>

namespace Engine {
class ENGINE_API GalaxyDistribution {
public:
    explicit GalaxyDistribution(float maxWidth, float division, float offset);

    [[nodiscard]] std::optional<Vector2> operator()(std::mt19937_64& rng);

    static void bind(Lua& lua);

private:
    float maxWidth;
    float division;

    std::unordered_set<uint64_t> positions;
    std::uniform_int_distribution<int> distArmIndex;
    std::uniform_real_distribution<float> distArmDistance;
    std::uniform_real_distribution<float> distArmOffset;
    std::uniform_real_distribution<float> distGridOffset;
};
} // namespace Engine
