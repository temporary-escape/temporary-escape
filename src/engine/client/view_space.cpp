#include "view_space.hpp"
#include "client.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

ViewSpace::ViewSpace(Game& parent, const Config& config, Renderer& renderer, Registry& registry, Skybox& skybox,
                     Client& client, Gui& gui) :
    parent{parent}, config{config}, registry{registry}, skyboxSystem{skybox}, client{client}, gui{gui} {
}

void ViewSpace::update(const float deltaTime) {
}

const Renderer::Options& ViewSpace::getRenderOptions() {
    static Renderer::Options options{};
    return options;
}

Scene& ViewSpace::getRenderScene() {
    auto scene = client.getScene();
    if (!scene) {
        EXCEPTION("No scene present!");
    }
    return *scene;
}

const Skybox& ViewSpace::getRenderSkybox() {
    return skyboxSystem;
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
