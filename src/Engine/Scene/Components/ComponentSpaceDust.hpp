#pragma once

#include "../../Graphics/SkyboxTextures.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentSpaceDust : public Component {
public:
    ComponentSpaceDust() = default;
    explicit ComponentSpaceDust(EntityId entity);
    COMPONENT_DEFAULTS(ComponentSpaceDust);
};
} // namespace Engine
