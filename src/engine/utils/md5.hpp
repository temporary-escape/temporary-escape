#pragma once

#include "../library.hpp"
#include <string>

namespace Engine {
extern ENGINE_API std::string md5sum(const void* data, size_t size);
}
