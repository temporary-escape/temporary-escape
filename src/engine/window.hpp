#pragma once

#include "user_input.hpp"

namespace Engine {
class ENGINE_API Window : public UserInput {
public:
    virtual ~Window() = default;

    virtual void render(const Vector2i& viewport, float deltaTime) = 0;
    virtual void eventWindowResized(const Vector2i& size) = 0;
};
} // namespace Engine
