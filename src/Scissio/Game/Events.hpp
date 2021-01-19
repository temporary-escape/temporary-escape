#pragma once

#include "../Scene/Entity.hpp"

namespace Scissio {
struct EventEntitySelected {
    EntityPtr entity = nullptr;
};

struct EventBuildMode {
    EntityPtr entity = nullptr;
};

struct EventSpaceMode {};
} // namespace Scissio
