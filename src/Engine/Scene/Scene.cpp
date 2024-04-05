#include "Scene.hpp"
#include "../Assets/AssetsManager.hpp"
#include "Controllers/ControllerBullets.hpp"
#include "Controllers/ControllerCamera.hpp"
#include "Controllers/ControllerCameraOrbital.hpp"
#include "Controllers/ControllerCameraPanning.hpp"
#include "Controllers/ControllerGrid.hpp"
#include "Controllers/ControllerIcon.hpp"
#include "Controllers/ControllerIconSelectable.hpp"
#include "Controllers/ControllerLights.hpp"
#include "Controllers/ControllerLines.hpp"
#include "Controllers/ControllerModel.hpp"
#include "Controllers/ControllerModelSkinned.hpp"
#include "Controllers/ControllerNetwork.hpp"
#include "Controllers/ControllerPathfinding.hpp"
#include "Controllers/ControllerPointCloud.hpp"
#include "Controllers/ControllerPolyShape.hpp"
#include "Controllers/ControllerRigidBody.hpp"
#include "Controllers/ControllerShipControl.hpp"
#include "Controllers/ControllerStaticModel.hpp"
#include "Controllers/ControllerText.hpp"
#include "Controllers/ControllerTurret.hpp"
#include "Controllers/ControllerWorldText.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Scene::Scene(const Config& config, VoxelShapeCache* voxelShapeCache, Lua* lua) :
    lua{lua}, dynamicsWorld{*this, reg, config} {

    addController<ControllerGrid>(dynamicsWorld, voxelShapeCache);
    addController<ControllerRigidBody>(dynamicsWorld);
    network = &addController<ControllerNetwork>();
    auto& bullets = addController<ControllerBullets>(dynamicsWorld);
    addController<ControllerTurret>(dynamicsWorld, bullets);
    addController<ControllerShipControl>();
    addController<ControllerModel>();
    // addController<ControllerPathfinding>(dynamicsWorld);

    if (voxelShapeCache) {
        addController<ControllerIconSelectable>();
        addController<ControllerIcon>();
        addController<ControllerCamera>();
        addController<ControllerCameraOrbital>();
        addController<ControllerCameraPanning>();
        addController<ControllerLights>();
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
    if (reg.valid(EntityId{id})) {
        selectedEntityOpaque = Entity{reg, EntityId{id}};
    } else {
        selectedEntityOpaque = std::nullopt;
    }
}

Entity Scene::createEntity() {
    return fromHandle(reg.create());
}

EntityId Scene::createEntityFrom(const std::string& name, const sol::table& data) {
    const auto it = entityTemplates.find(name);
    if (it == entityTemplates.end()) {
        throw std::runtime_error(fmt::format("No such entity template named: '{}'", name));
    }

    auto entity = fromHandle(reg.create());

    auto res = (*it->second)["new"](entity, data);
    if (!res.valid()) {
        sol::error err = res;
        throw std::runtime_error(fmt::format("Entity failed to create error: {}", err.what()));
    }

    auto inst = res.get<sol::table>();

    addComponent<ComponentScript>(entity.getHandle(), inst);
    return entity.getHandle();
}

EntityId Scene::createEntityFrom(const std::string& name) {
    return createEntityFrom(name, sol::table{});
}

void Scene::addEntityTemplate(const std::string& name, const sol::table& klass) {
    entityTemplates.emplace(name, std::make_unique<sol::table>(klass));
}

void Scene::removeEntity(Entity& entity) {
    reg.destroy(entity.getHandle());
    entity.reset();
}

Entity Scene::fromHandle(EntityId handle) {
    return Entity{reg, handle};
}

void Scene::update(const float delta) {
    // const auto t0 = std::chrono::steady_clock::now();

    dynamicsWorld.update(delta);

    // const auto t1 = std::chrono::steady_clock::now();
    // const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    // logger.info("dynamicsWorld update took: {} ms", ms);

    for (auto& [_, controller] : controllers) {
        controller->update(delta);
    }

    if (selectionEnabled) {
        updateSelection();
    }
}

void Scene::interpolate(const float delta) {
    const auto& entities = reg.view<ComponentTransform, ComponentShipControl>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, component] : entities) {
        transform.interpolate();
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
                setDirty(*icon);
            }
        }

        selectedEntityLast = selectedEntity;

        if (selectedEntityLast) {
            auto* icon = selectedEntityLast->tryGetComponent<ComponentIcon>();
            if (icon && icon->isEnvironment()) {
                auto color = icon->getColor();
                color.a = 1.0f;
                icon->setColor(color);
                setDirty(*icon);
            }
        }
    }
}

void Scene::recalculate(VulkanRenderer& vulkan) {
    dynamicsWorld.recalculate(vulkan);

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
    return dynamicsWorld.contactTestSphere(origin, radius);
}

float Scene::getEntityBounds(const EntityId a, const ComponentTransform& transform) const {
    const auto* model = tryGetComponent<ComponentModel>(a);
    if (model && model->getModel()) {
        return model->getModel()->getRadius() * transform.getScaleUniform();
    }

    const auto* grid = tryGetComponent<ComponentGrid>(a);
    if (grid) {
        return grid->getRadius();
    }

    return 0.0f;
}

float Scene::getEntityDistance(const EntityId a, const EntityId b) const {
    const auto* at = tryGetComponent<ComponentTransform>(a);
    if (!at) {
        return std::numeric_limits<float>::infinity();
    }

    const auto* bt = tryGetComponent<ComponentTransform>(b);
    if (!bt) {
        return std::numeric_limits<float>::infinity();
    }

    const auto dist = glm::distance(at->getAbsolutePosition(), bt->getAbsolutePosition()) - getEntityBounds(a, *at) -
                      getEntityBounds(b, *bt);

    return dist < 0.0f ? 0.0f : dist;
}

bool Scene::valid(const EntityId entity) const {
    return reg.valid(entity);
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

EntityId Scene::getRemoteId(const EntityId entity) const {
    return network->getLocalToRemote(entity);
}

EntityId Scene::getLocalId(const EntityId entity) const {
    return network->getRemoteToLocal(entity);
}
