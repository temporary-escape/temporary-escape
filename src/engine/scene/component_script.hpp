#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API ComponentScript : public Component {
public:
    ComponentScript() = default;
    virtual ~ComponentScript() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentScript);

private:
    // wrenbind17::Variable script;
};
} // namespace Engine
