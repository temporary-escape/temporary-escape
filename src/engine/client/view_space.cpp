#include "view_space.hpp"
#include "client.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ViewSpace::ViewSpace(Game& parent, const Config& config, Renderer& renderer, AssetsManager& assetsManager,
                     Skybox& skybox, Client& client) :
    parent{parent}, config{config}, assetsManager{assetsManager}, skyboxSystem{skybox}, client{client} {
}

void ViewSpace::update(const float deltaTime) {
    auto scene = client.getScene();
    if (const auto selected = scene->getSelectedEntity(); selected != selectedEntity) {
        if (selectedEntity) {
            auto* icon = selectedEntity->tryGetComponent<ComponentIcon>();
            if (icon) {
                icon->setColor(Vector4{0.7, 0.7, 0.7, 0.5});
                icon->setDirty(true);
            }
        }

        selectedEntity = selected;
        if (selectedEntity) {
            logger.info("Selected entity: {}", static_cast<uint32_t>(selectedEntity->getHandle()));

            if (selectedEntity) {
                auto* icon = selectedEntity->tryGetComponent<ComponentIcon>();
                if (icon) {
                    icon->setColor(Vector4{1.0, 0.0, 0.0, 1.0});
                    icon->setDirty(true);
                }
            }
        } else {
            logger.info("Selected entity: null");
        }
    }
}

Scene* ViewSpace::getScene() {
    auto scene = client.getScene();
    if (!scene) {
        EXCEPTION("No scene present!");
    }
    return scene;
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
