#include "view_space.hpp"
#include "../graphics/renderer.hpp"
#include "client.hpp"

#define CMP "ViewSpace"

using namespace Engine;

ViewSpace::ViewSpace(const Config& config, VulkanDevice& vulkan, Registry& registry, Skybox& skybox, Client& client) :
    config{config}, vulkan{vulkan}, registry{registry}, skyboxSystem{skybox}, client{client} {
}

void ViewSpace::update(const float deltaTime) {
}

void ViewSpace::render(const Vector2i& viewport, Renderer& renderer) {
    auto scene = client.getScene();
    if (scene) {
        Renderer::Options options{};
        options.blurStrength = 0.2f;
        renderer.render(viewport, *scene, skyboxSystem, options);
    }
}

void ViewSpace::renderCanvas(const Vector2i& viewport, Canvas& canvas) {
}

void ViewSpace::renderGui(const Vector2i& viewport, Nuklear& nuklear) {
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
