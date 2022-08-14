#pragma once

#include "../Input/UserInput.hpp"
#include "../Vulkan/Enums.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentUserInput : public Component, UserInput::Handler {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentUserInput() = default;
    explicit ComponentUserInput(Object& object, UserInput::Handler& handler) : Component(object), handler(&handler) {
    }
    virtual ~ComponentUserInput() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void eventUserInput(const UserInput::Event& event) override {
        if (handler && !disable) {
            handler->eventUserInput(event);
        }
    }

    void setDisabled(const bool value) {
        disable = value;
    }

    bool getDisabled() const {
        return disable;
    }

private:
    UserInput::Handler* handler{nullptr};
    bool disable{false};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
