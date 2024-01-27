#pragma once

#include "../Library.hpp"
#include <chrono>
#include <string>

namespace Engine {
ENGINE_API std::string timePointToIso(const std::chrono::system_clock::time_point& tp);
ENGINE_API std::chrono::system_clock::time_point isoToTimePoint(const std::string& str);
} // namespace Engine
