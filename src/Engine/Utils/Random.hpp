#pragma once

#include "../Library.hpp"
#include <functional>
#include <random>

namespace Engine {
using Rng = std::mt19937_64;

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
inline uint64_t randomSeed(std::mt19937_64& rng) {
    return randomInt<uint64_t>(rng, 0, std::numeric_limits<long long>::max());
}
template <typename Container, typename T = typename Container::value_type>
inline const T& randomPick(std::mt19937_64& rng, const Container& container) {
    return container.at(randomInt<size_t>(rng, 0, container.size() - 1));
}
} // namespace Engine
