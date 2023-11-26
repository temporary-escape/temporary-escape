#pragma once

#include "Enums.hpp"
#include "Library.hpp"
#include "Math/Vector.hpp"

namespace Engine {
class ENGINE_API UserInput {
public:
    virtual ~UserInput() = default;

    virtual void eventMouseMoved(const Vector2i& pos) = 0;
    virtual void eventMousePressed(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseReleased(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseScroll(int xscroll, int yscroll) = 0;
    virtual void eventKeyPressed(Key key, Modifiers modifiers) = 0;
    virtual void eventKeyReleased(Key key, Modifiers modifiers) = 0;
    virtual void eventCharTyped(uint32_t code) = 0;
};
} // namespace Engine
