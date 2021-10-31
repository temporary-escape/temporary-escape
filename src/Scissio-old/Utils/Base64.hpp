#pragma once

#include "../Library.hpp"
#include <string>
#include <vector>

namespace Scissio {
SCISSIO_API std::vector<uint8_t> base64Decode(const std::string_view& source);
} // namespace Scissio
