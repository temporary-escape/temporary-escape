#pragma once

#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../window.hpp"

namespace Engine {
class ENGINE_API Renderer;

class ENGINE_API View : public UserInput {
public:
    virtual ~View() = default;

    virtual void update(float deltaTime) = 0;
    virtual void render(const Vector2i& viewport) = 0;
    virtual void renderCanvas(const Vector2i& viewport) = 0;
    virtual void renderGui(const Vector2i& viewport) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
};
} // namespace Engine
