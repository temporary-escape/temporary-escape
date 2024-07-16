#pragma once

#include "../Library.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Engine {
using HkdfResult = std::array<uint8_t, 32>;
ENGINE_API HkdfResult hkdfKeyDerivation(const std::vector<uint8_t>& sharedSecret, const std::string_view& salt);
} // namespace Engine
