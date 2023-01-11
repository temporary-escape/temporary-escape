#pragma once

#include "../user_input.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentUserInput : public Component, UserInput {
public:
    ComponentUserInput() = default;
    explicit ComponentUserInput(UserInput& handler) : handler{&handler} {
    }
    virtual ~ComponentUserInput() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentUserInput);

    void eventMouseMoved(const Vector2i& pos) override;

    void eventMousePressed(const Vector2i& pos, MouseButton button) override;

    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;

    void eventMouseScroll(int xscroll, int yscroll) override;

    void eventKeyPressed(Key key, Modifiers modifiers) override;

    void eventKeyReleased(Key key, Modifiers modifiers) override;

    void eventCharTyped(uint32_t code) override;

    void setDisabled(const bool value) {
        disable = value;
    }

    [[nodiscard]] bool getDisabled() const {
        return disable;
    }

private:
    UserInput* handler{nullptr};
    bool disable{false};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
