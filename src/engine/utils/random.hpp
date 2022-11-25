#pragma once

#include "../library.hpp"
#include <functional>
#include <random>

namespace Engine {
ENGINE_API extern uint64_t randomId(const std::function<bool(uint64_t)>& pred = nullptr);
ENGINE_API extern std::string uuid();
template <typename T>
inline T randomInt(std::mt19937_64& rng, const T min = std::numeric_limits<T>::min(),
                   const T max = std::numeric_limits<T>::max()) {
    std::uniform_int_distribution<T> dist{min, max};
    return dist(rng);
}
template <typename T> inline T randomReal(std::mt19937_64& rng, const T min, const T max) {
    std::uniform_real_distribution<T> dist{min, max};
    return dist(rng);
}
} // namespace Engine
