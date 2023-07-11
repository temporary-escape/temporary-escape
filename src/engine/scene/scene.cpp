#include "scene.hpp"
#include "../assets/assets_manager.hpp"
#include "../server/lua.hpp"
#include "controllers/controller_2d_selectable.hpp"
#include "controllers/controller_camera.hpp"
#include "controllers/controller_dynamics_world.hpp"
#include "controllers/controller_icon.hpp"
#include "controllers/controller_network.hpp"
#include "controllers/controller_static_model.hpp"
#include "controllers/controller_text.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Scene::Scene(const Config& config, const bool isServer) {
    addController<ControllerDynamicsWorld>(config);
    addController<ControllerNetwork>();

    if (!isServer) {
        addController<Controller2DSelectable>();
        addController<ControllerIcon>();
        addController<ControllerCamera>();
        addController<ControllerStaticModel>();
        addController<ControllerText>();
    }
}

Scene::~Scene() = default;

Entity Scene::createEntity() {
    return fromHandle(reg.create());
}

void Scene::removeEntity(Entity& entity) {
    reg.destroy(entity.getHandle());
    entity.reset();
}

Entity Scene::fromHandle(entt::entity handle) {
    return Entity{reg, handle};
}

void Scene::update(const float delta) {
    for (auto& [_, controller] : controllers) {
        controller->update(delta);
    }

    /*auto& systemTurret = getComponentSystem<ComponentTurret>();
    for (auto& component : systemTurret) {
        component->update(delta);
        if (component->shouldFire()) {
            component->resetFire();
            addBullet(component->getNozzlePos(), component->getNozzleDir());
        }
    }*/
}

ComponentCamera* Scene::getPrimaryCamera() {
    if (primaryCamera) {
        if (!primaryCamera.hasComponent<ComponentCamera>()) {
            return nullptr;
        }

        return &primaryCamera.getComponent<ComponentCamera>();
    }

    return nullptr;
}

const SkyboxTextures* Scene::getSkybox() {
    for (auto&& [entity, skybox] : getView<ComponentSkybox>().each()) {
        if (!skybox.isGenerated() || !skybox.getTextures().getTexture()) {
            continue;
        }
        return &skybox.getTextures();
    }
    return nullptr;
}

void Scene::eventMouseMoved(const Vector2i& pos) {
    for (auto input : userInputs) {
        input->eventMouseMoved(pos);
    }
}

void Scene::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    for (auto input : userInputs) {
        input->eventMousePressed(pos, button);
    }
}

void Scene::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    for (auto input : userInputs) {
        input->eventMouseReleased(pos, button);
    }
}

void Scene::eventMouseScroll(const int xscroll, const int yscroll) {
    for (auto input : userInputs) {
        input->eventMouseScroll(xscroll, yscroll);
    }
}

void Scene::eventKeyPressed(const Key key, const Modifiers modifiers) {
    for (auto input : userInputs) {
        input->eventKeyPressed(key, modifiers);
    }
}

void Scene::eventKeyReleased(const Key key, const Modifiers modifiers) {
    for (auto input : userInputs) {
        input->eventKeyReleased(key, modifiers);
    }
}

void Scene::eventCharTyped(const uint32_t code) {
    for (auto input : userInputs) {
        input->eventCharTyped(code);
    }
}

std::tuple<Vector3, Vector3> Scene::screenToWorld(const Vector2& mousePos, float length) {
    const auto camera = getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Error during screenToWorld no camera in scene");
    }

    const auto x = static_cast<float>(camera->getViewport().x) - mousePos.x; // WTF?
    const auto y = mousePos.y;
    const auto ray = camera->screenToWorld({x, y}) * -length;
    const auto eyes = camera->getEyesPos();
    return {eyes, eyes + ray};
}

void Scene::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Scene>("Scene");
    cls["create_entity"] = &Scene::createEntity;
}
