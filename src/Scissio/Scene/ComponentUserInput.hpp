#pragma once

#include "../Platform/Enums.hpp"
#include "Component.hpp"

namespace Scissio {
class UserInputHandler {
public:
    virtual void eventMouseMoved(const Vector2i& pos) = 0;
    virtual void eventMousePressed(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseReleased(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseScroll(int xscroll, int yscroll) = 0;
    virtual void eventKeyPressed(Key key, Modifiers modifiers) = 0;
    virtual void eventKeyReleased(Key key, Modifiers modifiers) = 0;
};

class SCISSIO_API ComponentUserInput : public Component {
public:
    ComponentUserInput() = default;
    explicit ComponentUserInput(Object& object, UserInputHandler& handler) : Component(object), handler(&handler) {
    }
    virtual ~ComponentUserInput() = default;

    void eventMouseMoved(const Vector2i& pos) {
        if (handler && !disable) {
            handler->eventMouseMoved(pos);
        }
    }

    void eventMousePressed(const Vector2i& pos, MouseButton button) {
        if (handler && !disable) {
            handler->eventMousePressed(pos, button);
        }
    }

    void eventMouseReleased(const Vector2i& pos, MouseButton button) {
        if (handler && !disable) {
            handler->eventMouseReleased(pos, button);
        }
    }

    void eventMouseScroll(int xscroll, int yscroll) {
        if (handler && !disable) {
            handler->eventMouseScroll(xscroll, yscroll);
        }
    }

    void eventKeyPressed(Key key, Modifiers modifiers) {
        if (handler && !disable) {
            handler->eventKeyPressed(key, modifiers);
        }
    }

    void eventKeyReleased(Key key, Modifiers modifiers) {
        if (handler && !disable) {
            handler->eventKeyReleased(key, modifiers);
        }
    }

    void setDisabled(const bool value) {
        disable = value;
    }

    bool getDisabled() const {
        return disable;
    }

private:
    UserInputHandler* handler{nullptr};
    bool disable{false};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Scissio
