#pragma once

#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentPlayer : public Component {
public:
    ComponentPlayer() = default;
    explicit ComponentPlayer(EntityId entity, std::string playerId);
    COMPONENT_DEFAULTS(ComponentPlayer);

    [[nodiscard]] const std::string& getPlayerId() const {
        return playerId;
    }

private:
    std::string playerId;
};
} // namespace Engine
