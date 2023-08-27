#include "scene.hpp"
#include "../assets/assets_manager.hpp"
#include "../server/lua.hpp"
#include "controllers/controller_camera.hpp"
#include "controllers/controller_camera_orbital.hpp"
#include "controllers/controller_camera_panning.hpp"
#include "controllers/controller_dynamics_world.hpp"
#include "controllers/controller_grid.hpp"
#include "controllers/controller_icon.hpp"
#include "controllers/controller_icon_selectable.hpp"
#include "controllers/controller_lights.hpp"
#include "controllers/controller_lines.hpp"
#include "controllers/controller_model_skinned.hpp"
#include "controllers/controller_network.hpp"
#include "controllers/controller_point_cloud.hpp"
#include "controllers/controller_poly_shape.hpp"
#include "controllers/controller_static_model.hpp"
#include "controllers/controller_text.hpp"
#include "controllers/controller_world_text.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Scene::Scene(const Config& config, VoxelShapeCache* voxelShapeCache, Lua* lua) : lua{lua} {
    addController<ControllerDynamicsWorld>(config);
    addController<ControllerNetwork>();

    if (voxelShapeCache) {
        addController<ControllerIconSelectable>();
        addController<ControllerIcon>();
        addController<ControllerCamera>();
        addController<ControllerCameraOrbital>();
        addController<ControllerCameraPanning>();
        addController<ControllerGrid>(*voxelShapeCache);
        addController<ControllerLights>(*this);
        addController<ControllerModelSkinned>();
        addController<ControllerStaticModel>();
        addController<ControllerText>();
        addController<ControllerWorldText>();
        addController<ControllerPointCloud>();
        addController<ControllerPolyShape>();
        addController<ControllerLines>();
    }
}

Scene::~Scene() = default;

void Scene::feedbackSelectedEntity(const uint32_t id) {
    if (reg.valid(entt::entity{id})) {
        selectedEntityOpaque = Entity{reg, entt::entity{id}};
    } else {
        selectedEntityOpaque = std::nullopt;
    }
}

Entity Scene::createEntity() {
    return fromHandle(reg.create());
}

Entity Scene::createEntityFrom(const std::string& name, const sol::table& data) {
    const auto it = entityTemplates.find(name);
    if (it == entityTemplates.end()) {
        throw std::runtime_error(fmt::format("No such entity template named: '{}'", name));
    }

    auto entity = createEntity();

    auto res = (*it->second)["new"](entity, data);
    if (!res.valid()) {
        sol::error err = res;
        throw std::runtime_error(fmt::format("Entity failed to create error: {}", err.what()));
    }

    auto inst = res.get<sol::table>();

    entity.addComponent<ComponentScript>(inst);

    return entity;
}

void Scene::addEntityTemplate(const std::string& name, const sol::table& klass) {
    entityTemplates.emplace(name, std::make_unique<sol::table>(klass));
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

    if (selectionEnabled) {
        updateSelection();
    }
}

void Scene::updateSelection() {
    if (selectedEntityOpaque) {
        selectedEntity = selectedEntityOpaque;
    }

    if (selectedEntityIcon) {
        const auto* icon = selectedEntityIcon->tryGetComponent<ComponentIcon>();
        if (icon && !icon->isEnvironment()) {
            selectedEntity = selectedEntityIcon;
        }
    } else if (!selectedEntityOpaque) {
        selectedEntity = std::nullopt;
    }

    if (selectedEntityLast != selectedEntity) {
        if (selectedEntityLast) {
            auto* icon = selectedEntityLast->tryGetComponent<ComponentIcon>();
            if (icon && icon->isEnvironment()) {
                auto color = icon->getColor();
                color.a = 0.0f;
                icon->setColor(color);
            }
        }

        selectedEntityLast = selectedEntity;

        if (selectedEntityLast) {
            auto* icon = selectedEntityLast->tryGetComponent<ComponentIcon>();
            if (icon && icon->isEnvironment()) {
                auto color = icon->getColor();
                color.a = 1.0f;
                icon->setColor(color);
            }
        }
    }
}

void Scene::recalculate(VulkanRenderer& vulkan) {
    for (auto& [_, controller] : controllers) {
        controller->recalculate(vulkan);
    }

    if (hasController<ControllerIconSelectable>()) {
        const auto& iconsSelectable = getController<ControllerIconSelectable>();
        const auto selected = iconsSelectable.getSelected();
        if (selected) {
            selectedEntityIcon = Entity{reg, *selected};
        } else {
            selectedEntityIcon = std::nullopt;
        }
    }
}

ComponentCamera* Scene::getPrimaryCamera() const {
    if (primaryCamera) {
        if (!primaryCamera.hasComponent<ComponentCamera>()) {
            return nullptr;
        }

        return &primaryCamera.getComponent<ComponentCamera>();
    }

    return nullptr;
}

const ComponentSkybox* Scene::getSkybox() {
    for (auto&& [entity, skybox] : getView<ComponentSkybox>().each()) {
        if (!skybox.isGenerated() || !skybox.getTextures().getTexture()) {
            continue;
        }
        return &skybox;
    }
    return nullptr;
}

Lua& Scene::getLua() const {
    if (!lua) {
        EXCEPTION("No lua state within the scene");
    }
    return *lua;
}

bool Scene::contactTestSphere(const Vector3& origin, const float radius) const {
    const auto& dynamicsWorld = getController<ControllerDynamicsWorld>();
    return dynamicsWorld.contactTestSphere(origin, radius);
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

std::tuple<Vector3, Vector3> Scene::screenToWorld(const Vector2& mousePos, float length) const {
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

Vector2 Scene::worldToScreen(const Vector3& pos) const {
    const auto camera = getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Error during worldToScreen no camera in scene");
    }

    return camera->worldToScreen(pos, true);
}

void Scene::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Scene>("Scene");
    cls["create_entity"] = &Scene::createEntity;
    cls["contact_test_sphere"] = &Scene::contactTestSphere;
    cls["add_entity_template"] = &Scene::addEntityTemplate;
    cls["create_entity_template"] = &Scene::createEntityFrom;
}
