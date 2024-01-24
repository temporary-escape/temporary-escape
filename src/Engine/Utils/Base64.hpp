#pragma once

#include "../Library.hpp"
#include <string>
#include <vector>

namespace Engine {
ENGINE_API std::vector<uint8_t> base64Decode(const std::string_view& source);
ENGINE_API std::string base64Encode(const void* data, size_t size, bool url = false);
} // namespace Engine
