#include "scene.hpp"
#include "../assets/registry.hpp"

#define CMP "Scene"

using namespace Engine;

Scene::Scene(EventListener& eventListener) : eventListener{eventListener}, nextId{0} {
}

Scene::~Scene() = default;

/*void Scene::renderFwd(VulkanDevice& vulkan, const Vector2i& viewport) {
    using VariantComponent = std::variant<ComponentDebug*, ComponentWireframe*, ComponentPointCloud*, ComponentLines*,
                                          ComponentIconPointCloud*>;

    struct Renderable {
        VariantComponent cmp;
        uint64_t order{0};
    };

    if (!pipelines) {
        EXCEPTION("Scene shaders not initialized");
    }

    const auto camera = getPrimaryCamera();

    std::vector<Renderable> sorted;

    // TODO: Cleanup with variadic templates

    for (auto component : getComponentSystem<ComponentDebug>()) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        sorted.emplace_back(Renderable{component, component->getRenderOrder()});
    }
    for (auto component : getComponentSystem<ComponentWireframe>()) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        sorted.emplace_back(Renderable{component, component->getRenderOrder()});
    }
    for (auto component : getComponentSystem<ComponentPointCloud>()) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        sorted.emplace_back(Renderable{component, component->getRenderOrder()});
    }
    for (auto component : getComponentSystem<ComponentLines>()) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        sorted.emplace_back(Renderable{component, component->getRenderOrder()});
    }
    for (auto component : getComponentSystem<ComponentIconPointCloud>()) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        sorted.emplace_back(Renderable{component, component->getRenderOrder()});
    }

    std::sort(sorted.begin(), sorted.end(), [](Renderable& a, Renderable& b) -> bool { return a.order > b.order; });

    const auto setupRenderDebug = [this, &camera, &vulkan]() {
        vulkan.bindPipeline(pipelines->debug);
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
    };

    const auto setupRenderWireframe = [this, &camera, &vulkan]() {
        vulkan.bindPipeline(pipelines->wireframe);
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
    };

    const auto setupRenderPointCloud = [this, &camera, &vulkan]() {
        vulkan.bindPipeline(pipelines->pointCloud);
        vulkan.bindUniformBuffer(camera->getUbo(), 0);
        vulkan.setDepthStencilState(false, true);
        VulkanBlendState blendState{};
        blendState.blendEnable = true;
        blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        vulkan.setBlendState({blendState});
    };

    const auto setupRenderLines = [this, &camera, &vulkan]() {
        vulkan.bindPipeline(pipelines->lines);
        vulkan.bindUniformBuffer(camera->getUbo(), 0);
        vulkan.setDepthStencilState(false, true);
        VulkanBlendState blendState{};
        blendState.blendEnable = true;
        blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        vulkan.setBlendState({blendState});
    };

    const auto setupRenderIconPointCloud = [this, &camera, &vulkan]() {
        vulkan.bindPipeline(pipelines->iconPointCloud);
        vulkan.bindUniformBuffer(camera->getUbo(), 0);
        vulkan.setDepthStencilState(false, true);
        VulkanBlendState blendState{};
        blendState.blendEnable = true;
        blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
        blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
        blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        vulkan.setBlendState({blendState});
    };

    size_t currentIndex = -1;

    for (const auto& renderable : sorted) {
        const auto& variant = renderable.cmp;

        if (variant.index() == 0) {
            auto& component = *std::get<ComponentDebug*>(variant);

            if (currentIndex != 0) {
                setupRenderDebug();
                currentIndex = 0;
            }

            component.render(vulkan, viewport, pipelines->debug);
        } else if (variant.index() == 1) {
            auto& component = *std::get<ComponentWireframe*>(variant);

            if (currentIndex != 1) {
                setupRenderWireframe();
                currentIndex = 1;
            }

            component.render(vulkan, viewport, pipelines->wireframe);
        } else if (variant.index() == 2) {
            auto& component = *std::get<ComponentPointCloud*>(variant);

            if (currentIndex != 2) {
                setupRenderPointCloud();
                currentIndex = 2;
            }

            component.render(vulkan, viewport, pipelines->pointCloud);
        } else if (variant.index() == 3) {
            auto& component = *std::get<ComponentLines*>(variant);

            if (currentIndex != 3) {
                setupRenderLines();
                currentIndex = 3;
            }

            component.render(vulkan, viewport, pipelines->lines);
        } else if (variant.index() == 4) {
            auto& component = *std::get<ComponentIconPointCloud*>(variant);

            if (currentIndex != 4) {
                setupRenderIconPointCloud();
                currentIndex = 4;
            }

            component.render(vulkan, viewport, pipelines->iconPointCloud);
        }
    }*/

/*{
    if (!pipelines->debug) {
        EXCEPTION("Scene shader debug not initialized");
    }

    auto& system = getComponentSystem<ComponentDebug>();

    vulkan.bindPipeline(pipelines->debug);
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
        component->render(vulkan, viewport, pipelines->debug);
    }
}

{
    if (!pipelines->wireframe) {
        EXCEPTION("Scene shader wireframe not initialized");
    }

    auto& system = getComponentSystem<ComponentWireframe>();

    vulkan.bindPipeline(pipelines->wireframe);
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
        component->render(vulkan, viewport, pipelines->wireframe);
    }
}
}*/

EntityPtr Scene::createEntity() {
    auto entity = std::make_shared<Entity>(reg);
    entities.push_back(entity);
    return entity;

    /*if (entity->getId() == 0) {
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
    }*/
}

void Scene::removeEntity(const EntityPtr& entity) {
    if (!entity) {
        return;
    }

    entity->destroy();
    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());

    /*for (const auto& child : entity->getChildren()) {
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
    }*/
}

EntityPtr Scene::getEntityById(uint64_t id) {
    const auto it = entityMap.find(id);
    if (it == entityMap.end()) {
        return nullptr;
    }
    return it->second;
}

void Scene::addBullet(const Vector3& pos, const Vector3& dir) {
    /*const auto add = [&](Bullet& bullet) {
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
    add(bullets.data.at(endIdx));*/
}

void Scene::update(const float delta) {
    for (auto&& [entity, camera] : getView<ComponentCamera>().each()) {
        camera.update(delta);
    }

    /*auto& systemCamera = getComponentSystem<ComponentCamera>();
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
    }*/
}

ComponentCamera* Scene::getPrimaryCamera() {
    if (auto ptr = primaryCamera.lock(); ptr != nullptr) {
        if (!ptr->hasComponent<ComponentCamera>()) {
            return nullptr;
        }

        return &ptr->getComponent<ComponentCamera>();
    }

    return nullptr;
}

void Scene::eventMouseMoved(const Vector2i& pos) {
    auto componentSystemUserInput = getView<ComponentUserInput>();
    for (auto&& [entity, input] : componentSystemUserInput.each()) {
        input.eventMouseMoved(pos);
    }
}

void Scene::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    auto componentSystemUserInput = getView<ComponentUserInput>();
    for (auto&& [entity, input] : componentSystemUserInput.each()) {
        input.eventMousePressed(pos, button);
    }
}

void Scene::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto componentSystemUserInput = getView<ComponentUserInput>();
    for (auto&& [entity, input] : componentSystemUserInput.each()) {
        input.eventMouseReleased(pos, button);
    }
}

void Scene::eventMouseScroll(const int xscroll, const int yscroll) {
    auto componentSystemUserInput = getView<ComponentUserInput>();
    for (auto&& [entity, input] : componentSystemUserInput.each()) {
        input.eventMouseScroll(xscroll, yscroll);
    }
}

void Scene::eventKeyPressed(const Key key, const Modifiers modifiers) {
    auto componentSystemUserInput = getView<ComponentUserInput>();
    for (auto&& [entity, input] : componentSystemUserInput.each()) {
        input.eventKeyPressed(key, modifiers);
    }
}

void Scene::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto componentSystemUserInput = getView<ComponentUserInput>();
    for (auto&& [entity, input] : componentSystemUserInput.each()) {
        input.eventKeyReleased(key, modifiers);
    }
}

void Scene::eventCharTyped(const uint32_t code) {
    auto componentSystemUserInput = getView<ComponentUserInput>();
    for (auto&& [entity, input] : componentSystemUserInput.each()) {
        input.eventCharTyped(code);
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
