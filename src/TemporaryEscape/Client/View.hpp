#pragma once

#include "../Input/UserInput.hpp"
#include "../Vulkan/Window.hpp"
#include "Skybox.hpp"

namespace Engine {
class View : public UserInput::Handler {
public:
    virtual ~View() = default;

    virtual void update(float deltaTime) = 0;
    virtual void renderPbr(const Vector2i& viewport) = 0;
    virtual void renderFwd(const Vector2i& viewport) = 0;
    virtual void renderCanvas(const Vector2i& viewport) = 0;
    virtual const Skybox* getSkybox() = 0;
};
} // namespace Engine
