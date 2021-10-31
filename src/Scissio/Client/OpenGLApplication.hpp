#pragma once

#include "../Platform/OpenGLWindow.hpp"
#include "Application.hpp"

namespace Scissio {
class SCISSIO_API OpenGLApplication : public OpenGLWindow {
public:
    OpenGLApplication(Config& config);
    ~OpenGLApplication() override;

    void render(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    Application application;
};
} // namespace Scissio
