#include "Scene.hpp"
#include "../Assets/Registry.hpp"

#define CMP "Scene"

using namespace Engine;

Scene::Scene(EventListener& eventListener) :
    eventListener{eventListener}, nextId{0}, systems{EntityComponentHelper::generateComponentSystemsMap()} {
}

Scene::~Scene() = default;

void Scene::renderPbr(VulkanDevice& vulkan, const Vector2i& viewport, Pipelines& pipelines,
                      const VoxelShapeCache& voxelShapeCache) {
    const auto camera = getPrimaryCamera();
    camera->render(vulkan, viewport);

    {
        auto& system = getComponentSystem<ComponentGrid>();

        for (auto& component : system) {
            component->recalculate(vulkan, voxelShapeCache);
        }
    }

    {
        if (!pipelines.grid) {
            EXCEPTION("Scene shader grid not initialized");
        }

        auto& system = getComponentSystem<ComponentGrid>();

        vulkan.bindPipeline(pipelines.grid);
        vulkan.bindUniformBuffer(camera->getUbo(), 0);
        vulkan.setDepthStencilState(true, true);
        VulkanBlendState blendState{};
        blendState.blendEnable = true;
        blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
        blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
        blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        vulkan.setBlendState({blendState});

        for (auto& component : system) {
            if (!component->getObject().isVisible()) {
                continue;
            }
            component->render(vulkan, viewport, pipelines.debug);
        }
    }
}

void Scene::renderFwd(VulkanDevice& vulkan, const Vector2i& viewport, Pipelines& pipelines) {
    const auto camera = getPrimaryCamera();

    {
        if (!pipelines.debug) {
            EXCEPTION("Scene shader debug not initialized");
        }

        auto& system = getComponentSystem<ComponentDebug>();

        vulkan.bindPipeline(pipelines.debug);
        vulkan.bindUniformBuffer(camera->getUbo(), 0);
        vulkan.setDepthStencilState(true, true);
        VulkanBlendState blendState{};
        blendState.blendEnable = true;
        blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
        blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
        blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        vulkan.setBlendState({blendState});

        for (auto& component : system) {
            if (!component->getObject().isVisible()) {
                continue;
            }
            component->render(vulkan, viewport, pipelines.debug);
        }
    }

    {
        if (!pipelines.wireframe) {
            EXCEPTION("Scene shader wireframe not initialized");
        }

        auto& system = getComponentSystem<ComponentWireframe>();

        vulkan.bindPipeline(pipelines.wireframe);
        vulkan.bindUniformBuffer(camera->getUbo(), 0);
        vulkan.setDepthStencilState(true, true);
        VulkanBlendState blendState{};
        blendState.blendEnable = true;
        blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
        blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
        blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        vulkan.setBlendState({blendState});

        for (auto& component : system) {
            if (!component->getObject().isVisible()) {
                continue;
            }
            component->render(vulkan, viewport, pipelines.wireframe);
        }
    }
}

void Scene::addEntity(EntityPtr entity) {
    if (entity->getId() == 0) {
        entity->setId(++nextId);
    } else {
        nextId = std::max(entity->getId(), nextId);
    }

    if (entity->getParentId()) {
        const auto found = entityMap.find(entity->getParentId());
        if (found != entityMap.end()) {
            entity->setParent(found->second);
        } else {
            Log::w(CMP, "Entity parent id: {} not found", entity->getParentId());
        }
    }

    entities.push_back(std::move(entity));
    entityMap.insert(std::make_pair(entities.back()->getId(), entities.back()));

    for (const auto& ref : entities.back()->getComponents()) {
        auto& system = systems.at(ref.type);
        system->add(*ref.ptr);
    }

    eventListener.eventEntityAdded(entities.back());

    for (const auto& child : entities.back()->getChildren()) {
        addEntity(child);
    }
}

void Scene::updateEntity(const Entity::Delta& delta) {
    auto it = entityMap.find(delta.id);
    if (it != entityMap.end()) {
        it->second->applyDelta(delta);
    }
}

void Scene::removeEntity(const EntityPtr& entity) {
    if (!entity) {
        return;
    }

    for (const auto& child : entity->getChildren()) {
        child->setParent(nullptr);
        removeEntity(child);
    }
    entity->clearChildren();
    entity->setParent(nullptr);
    if (entity->hasComponent<ComponentScript>()) {
        // entity->getComponent<ComponentScript>()->clear();
    }

    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
    entityMap.erase(entity->getId());
    for (const auto& ref : entity->getComponents()) {
        auto& system = systems.at(ref.type);
        system->remove(*ref.ptr);
    }
}

EntityPtr Scene::getEntityById(uint64_t id) {
    const auto it = entityMap.find(id);
    if (it == entityMap.end()) {
        return nullptr;
    }
    return it->second;
}

void Scene::addBullet(const Vector3& pos, const Vector3& dir) {
    const auto add = [&](Bullet& bullet) {
        bullet.pos = pos;
        bullet.dir = dir;
        bullet.time = 1.0f;
        bullet.color = Color4{1.0f};
        bullet.speed = 125.0f;
    };

    for (auto& bullet : bullets.data) {
        if (bullet.speed <= 0.0001f) {
            add(bullet);
            return;
        }
    }

    const auto endIdx = bullets.data.size();
    Log::d(CMP, "Resizing bullets buffer to {}", bullets.data.size() + 128);
    bullets.data.resize(bullets.data.size() + 128);
    add(bullets.data.at(endIdx));
}

void Scene::update(const float delta) {
    auto& systemCamera = getComponentSystem<ComponentCamera>();
    for (auto& component : systemCamera) {
        component->update(delta);
    }

    auto& systemShipControl = getComponentSystem<ComponentShipControl>();
    for (auto& component : systemShipControl) {
        component->update(delta);
    }

    auto& systemTurret = getComponentSystem<ComponentTurret>();
    for (auto& component : systemTurret) {
        component->update(delta);
        if (component->shouldFire()) {
            component->resetFire();
            addBullet(component->getNozzlePos(), component->getNozzleDir());
        }
    }

    for (auto& bullet : bullets.data) {
        if (bullet.speed <= 0.0001f) {
            continue;
        }

        bullet.time += delta;
        bullet.pos += bullet.dir * bullet.speed * delta;

        if (bullet.time > 10.0f) {
            bullet.speed = 0.0f;
        }
    }
}

std::shared_ptr<ComponentCamera> Scene::getPrimaryCamera() const {
    if (auto ptr = primaryCamera.lock(); ptr != nullptr) {
        const auto camera = ptr->findComponent<ComponentCamera>();
        if (camera) {
            return camera;
        }
    }

    return nullptr;
}

EntityPtr Scene::getSkybox() {
    auto& skyboxSystem = getComponentSystem<ComponentSkybox>();
    if (skyboxSystem.begin() != skyboxSystem.end()) {
        auto& component = **skyboxSystem.begin();
        return component.getObject().asEntity();
    }
    return nullptr;
}

void Scene::eventUserInput(const UserInput::Event& event) {
    auto& componentSystemUserInput = getComponentSystem<ComponentUserInput>();
    for (auto& component : componentSystemUserInput) {
        component->eventUserInput(event);
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
