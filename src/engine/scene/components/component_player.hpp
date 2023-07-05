#pragma once

#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentPlayer : public Component {
public:
    ComponentPlayer() = default;
    explicit ComponentPlayer(entt::registry& reg, entt::entity handle, std::string playerId);
    ~ComponentPlayer() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPlayer);

    [[nodiscard]] const std::string& getPlayerId() const {
        return playerId;
    }

private:
    std::string playerId;
};
} // namespace Engine
