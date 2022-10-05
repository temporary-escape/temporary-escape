#pragma once

#include "../Graphics/Skybox.hpp"
#include "../Input/UserInput.hpp"
#include "../Vulkan/Window.hpp"

namespace Engine {
class ENGINE_API Renderer;

class ENGINE_API View : public UserInput::Handler {
public:
    virtual ~View() = default;

    virtual void update(float deltaTime) = 0;
    virtual void render(const Vector2i& viewport, Renderer& renderer) = 0;
    virtual void renderCanvas(const Vector2i& viewport) = 0;
    virtual const Skybox* getSkybox() = 0;
};
} // namespace Engine
