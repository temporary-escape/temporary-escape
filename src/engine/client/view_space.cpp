#include "view_space.hpp"
#include "../graphics/renderer.hpp"
#include "client.hpp"

#define CMP "ViewSpace"

using namespace Engine;

ViewSpace::ViewSpace(const Config& config, Renderer& renderer, Registry& registry, Skybox& skybox, Client& client) :
    config{config}, renderer{renderer}, registry{registry}, skyboxSystem{skybox}, client{client} {
}

void ViewSpace::update(const float deltaTime) {
}

void ViewSpace::render(const Vector2i& viewport) {
    auto scene = client.getScene();
    if (!scene) {
        EXCEPTION("No scene present!");
    }

    Renderer::Options options{};
    renderer.render(viewport, *scene, skyboxSystem, options);
}

void ViewSpace::onEnter() {
}

void ViewSpace::onExit() {
}

void ViewSpace::eventMouseMoved(const Vector2i& pos) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseMoved(pos);
    }
}

void ViewSpace::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewSpace::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewSpace::eventMouseScroll(const int xscroll, const int yscroll) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewSpace::eventKeyPressed(const Key key, const Modifiers modifiers) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewSpace::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventKeyReleased(key, modifiers);
    }
}

void ViewSpace::eventCharTyped(const uint32_t code) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventCharTyped(code);
    }
}
