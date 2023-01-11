#include "component_user_input.hpp"

using namespace Engine;

void ComponentUserInput::eventMouseMoved(const Vector2i& pos) {
    if (handler && !disable) {
        handler->eventMouseMoved(pos);
    }
}

void ComponentUserInput::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (handler && !disable) {
        handler->eventMousePressed(pos, button);
    }
}

void ComponentUserInput::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (handler && !disable) {
        handler->eventMouseReleased(pos, button);
    }
}

void ComponentUserInput::eventMouseScroll(const int xscroll, const int yscroll) {
    if (handler && !disable) {
        handler->eventMouseScroll(xscroll, yscroll);
    }
}

void ComponentUserInput::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (handler && !disable) {
        handler->eventKeyPressed(key, modifiers);
    }
}

void ComponentUserInput::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (handler && !disable) {
        handler->eventKeyReleased(key, modifiers);
    }
}

void ComponentUserInput::eventCharTyped(const uint32_t code) {
    if (handler && !disable) {
        handler->eventCharTyped(code);
    }
}
