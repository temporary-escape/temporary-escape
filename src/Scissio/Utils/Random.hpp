#pragma once

#include "../Library.hpp"
#include <functional>
#include <random>

namespace Scissio {
SCISSIO_API extern uint64_t randomId(const std::function<bool(uint64_t)>& pred = nullptr);
SCISSIO_API extern std::string randomName(std::mt19937_64& rng);
} // namespace Scissio
