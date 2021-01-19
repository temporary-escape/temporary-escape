#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "Enums.hpp"

namespace Scissio {
class SCISSIO_API Window {
public:
    virtual ~Window() = default;

    virtual void render(const Vector2i& viewport) = 0;
    virtual void eventMouseMoved(const Vector2i& pos) = 0;
    virtual void eventMousePressed(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseReleased(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventKeyPressed(Key key) = 0;
    virtual void eventKeyReleased(Key key) = 0;
};
} // namespace Scissio
