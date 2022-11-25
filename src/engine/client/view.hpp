#pragma once

#include "../graphics/skybox.hpp"
#include "../input/user_input.hpp"
#include "../vulkan/window.hpp"

namespace Engine {
class ENGINE_API Renderer;

class ENGINE_API View : public UserInput::Handler {
public:
    virtual ~View() = default;

    virtual void update(float deltaTime) = 0;
    virtual void render(const Vector2i& viewport, Renderer& renderer) = 0;
    virtual void renderCanvas(const Vector2i& viewport) = 0;
};
} // namespace Engine
