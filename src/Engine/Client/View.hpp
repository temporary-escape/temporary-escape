#pragma once

#include "../Graphics/Nuklear.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Window.hpp"

namespace Engine {
class ENGINE_API Renderer;

class ENGINE_API View : public UserInput {
public:
    virtual ~View() = default;

    virtual void update(float deltaTime) = 0;
    virtual void renderCanvas(Canvas& canvas, const Vector2i& viewport) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual Scene* getScene() = 0;
};
} // namespace Engine
