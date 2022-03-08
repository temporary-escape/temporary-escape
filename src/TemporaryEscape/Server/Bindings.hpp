#pragma once

#include "../Config.hpp"
#include "../Library.hpp"

namespace wrenbind17 {
class VM;
}

namespace Engine {
extern ENGINE_API void expose(const Config& config, wrenbind17::VM& vm);
extern ENGINE_API void unexpose(const Config& config, wrenbind17::VM& vm);
} // namespace Engine
