#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Platform/Enums.hpp"

namespace Scissio {
class SCISSIO_API View {
public:
    virtual ~View() = default;

    virtual void update(const Vector2i& viewport) = 0;
    virtual void render(const Vector2i& viewport, Renderer& renderer) = 0;
    virtual void renderCanvas(const Vector2i& viewport, Canvas2D& canvas, GuiContext& gui) = 0;
    virtual void eventMouseMoved(const Vector2i& pos) = 0;
    virtual void eventMousePressed(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseReleased(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventKeyPressed(Key key, Modifiers modifiers) = 0;
    virtual void eventKeyReleased(Key key, Modifiers modifiers) = 0;
    virtual void eventMouseScroll(int xscroll, int yscroll) = 0;
};
} // namespace Scissio
