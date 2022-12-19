#pragma once

#include "../user_input.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentUserInput : public Component, UserInput {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentUserInput() = default;
    explicit ComponentUserInput(Object& object, UserInput& handler) : Component{object}, handler{&handler} {
    }
    virtual ~ComponentUserInput() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void eventMouseMoved(const Vector2i& pos) override {
        if (handler && !disable) {
            handler->eventMouseMoved(pos);
        }
    }

    void eventMousePressed(const Vector2i& pos, const MouseButton button) override {
        if (handler && !disable) {
            handler->eventMousePressed(pos, button);
        }
    }

    void eventMouseReleased(const Vector2i& pos, const MouseButton button) override {
        if (handler && !disable) {
            handler->eventMouseReleased(pos, button);
        }
    }

    void eventMouseScroll(const int xscroll, const int yscroll) override {
        if (handler && !disable) {
            handler->eventMouseScroll(xscroll, yscroll);
        }
    }

    void eventKeyPressed(const Key key, const Modifiers modifiers) override {
        if (handler && !disable) {
            handler->eventKeyPressed(key, modifiers);
        }
    }

    void eventKeyReleased(const Key key, const Modifiers modifiers) override {
        if (handler && !disable) {
            handler->eventKeyReleased(key, modifiers);
        }
    }

    void eventCharTyped(const uint32_t code) override {
        if (handler && !disable) {
            handler->eventCharTyped(code);
        }
    }

    void setDisabled(const bool value) {
        disable = value;
    }

    bool getDisabled() const {
        return disable;
    }

private:
    UserInput* handler{nullptr};
    bool disable{false};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
