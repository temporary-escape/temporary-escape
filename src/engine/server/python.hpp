#pragma once

#include "../utils/path.hpp"
#include <vector>

namespace Engine {
class Python {
public:
    explicit Python(const Path& home, const std::vector<Path>& paths);
    ~Python();
};
} // namespace Engine
