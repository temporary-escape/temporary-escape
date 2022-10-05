#pragma once

#include "../Library.hpp"
#include "../Utils/Random.hpp"
#include "Vector.hpp"
#include <vector>

namespace Engine {
std::vector<Vector2> ENGINE_API randomCirclePositions(std::mt19937_64& rng, float maxRadius, size_t maxCount,
                                                      float minDistance);
}
