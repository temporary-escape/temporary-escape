#pragma once

#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentPlayer : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentPlayer() = default;
    explicit ComponentPlayer(Object& object, std::string playerId) : Component(object), playerId(std::move(playerId)) {
    }
    virtual ~ComponentPlayer() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    const std::string& getPlayerId() const {
        return playerId;
    }

private:
    std::string playerId;

public:
    MSGPACK_DEFINE_ARRAY(playerId);
};
} // namespace Engine
