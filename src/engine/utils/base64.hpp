#pragma once

#include "../library.hpp"
#include <string>
#include <vector>

namespace Engine {
ENGINE_API std::vector<uint8_t> base64Decode(const std::string_view& source);
} // namespace Engine
