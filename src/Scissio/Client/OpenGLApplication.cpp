#include "OpenGLApplication.hpp"

using namespace Scissio;

OpenGLApplication::OpenGLApplication(Config& config) : OpenGLWindow("Scissio Game", {1920, 1080}), application(config) {
}

OpenGLApplication::~OpenGLApplication() {
}

void OpenGLApplication::render(const Vector2i& viewport) {
    application.render(viewport);
}

void OpenGLApplication::eventMouseMoved(const Vector2i& pos) {
    application.eventMouseMoved(pos);
}

void OpenGLApplication::eventMousePressed(const Vector2i& pos, MouseButton button) {
    application.eventMousePressed(pos, button);
}

void OpenGLApplication::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    application.eventMouseReleased(pos, button);
}

void OpenGLApplication::eventMouseScroll(const int xscroll, const int yscroll) {
    application.eventMouseScroll(xscroll, yscroll);
}

void OpenGLApplication::eventKeyPressed(Key key, Modifiers modifiers) {
    application.eventKeyPressed(key, modifiers);
}

void OpenGLApplication::eventKeyReleased(Key key, Modifiers modifiers) {
    application.eventKeyReleased(key, modifiers);
}
