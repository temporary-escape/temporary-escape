#include "quaternion.hpp"
#include <cmath>

using namespace Engine;

static const float PI = static_cast<float>(std::atan(1) * 4);

Quaternion Engine::randomQuaternion(std::mt19937_64& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const auto u = dist(rng);
    const auto v = dist(rng);
    const auto w = dist(rng);

    Quaternion q;
    auto* data = const_cast<float*>(&q.x);

    data[0] = std::sqrt(1.0f - u) * std::sin(2.0f * PI * v);
    data[1] = std::sqrt(1.0f - u) * std::cos(2.0f * PI * v);
    data[2] = std::sqrt(u) * std::sin(2.0f * PI * w);
    data[3] = std::sqrt(u) * std::cos(2.0f * PI * w);

    return q;
}
