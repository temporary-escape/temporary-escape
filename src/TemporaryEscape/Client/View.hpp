#pragma once

#include "../Math/Vector.hpp"
#include "../Platform/Enums.hpp"
#include "Renderer.hpp"

namespace Engine {
class Client;

class ENGINE_API View {
public:
    virtual void render(const Vector2i& viewport) = 0;
    virtual void eventMouseMoved(const Vector2i& pos) = 0;
    virtual void eventMousePressed(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseReleased(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseScroll(int xscroll, int yscroll) = 0;
    virtual void eventKeyPressed(Key key, Modifiers modifiers) = 0;
    virtual void eventKeyReleased(Key key, Modifiers modifiers) = 0;
};
} // namespace Engine
