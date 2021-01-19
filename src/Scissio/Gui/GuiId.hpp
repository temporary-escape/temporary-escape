#pragma once

#include <string>

namespace Scissio {
class GuiId {
public:
    GuiId() {
        id = std::to_string(reinterpret_cast<uint64_t>(this));
    }

    operator const char*() const {
        return id.c_str();
    }

private:
    std::string id;
};
} // namespace Scissio
