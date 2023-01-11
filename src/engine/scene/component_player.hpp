#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPlayer : public Component {
public:
    ComponentPlayer() = default;
    explicit ComponentPlayer(std::string playerId) : playerId{std::move(playerId)} {
    }
    virtual ~ComponentPlayer() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPlayer);

    [[nodiscard]] const std::string& getPlayerId() const {
        return playerId;
    }

private:
    std::string playerId;
};
} // namespace Engine
