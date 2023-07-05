#include "controller_dynamics_world.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class CollisionDebugDraw : public btIDebugDraw {
public:
    explicit CollisionDebugDraw(VulkanRenderer& vulkan) : vulkan{vulkan} {
    }
    ~CollisionDebugDraw() override = default;

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override {
        if (capacity < count + 2) {
            reserve();
        }
    }

private:
    void reserve() {
        capacity += 1024;

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(ComponentLines::Line) * capacity;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vbo = vulkan.createDoubleBuffer(bufferInfo);
    }

    VulkanRenderer& vulkan;
    VulkanDoubleBuffer vbo;
    size_t capacity{0};
    size_t count{0};
};

ControllerDynamicsWorld::ControllerDynamicsWorld(entt::registry& reg) :
    reg{reg},
    collisionConfiguration{std::make_unique<btDefaultCollisionConfiguration>()},
    dispatcher{std::make_unique<btCollisionDispatcher>(collisionConfiguration.get())},
    overlappingPairCache{std::make_unique<btDbvtBroadphase>()},
    solver{std::make_unique<btSequentialImpulseConstraintSolver>()},
    dynamicsWorld{std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), overlappingPairCache.get(), solver.get(),
                                                            collisionConfiguration.get())} {

    dynamicsWorld->setGravity(btVector3{0, 0, 0});

    reg.on_construct<ComponentRigidBody>().connect<&ControllerDynamicsWorld::onConstruct>(this);
    reg.on_update<ComponentRigidBody>().connect<&ControllerDynamicsWorld::onUpdate>(this);
    reg.on_destroy<ComponentRigidBody>().connect<&ControllerDynamicsWorld::onDestroy>(this);
}

ControllerDynamicsWorld::~ControllerDynamicsWorld() {
    reg.on_construct<ComponentRigidBody>().disconnect<&ControllerDynamicsWorld::onConstruct>(this);
    reg.on_update<ComponentRigidBody>().disconnect<&ControllerDynamicsWorld::onUpdate>(this);
    reg.on_destroy<ComponentRigidBody>().disconnect<&ControllerDynamicsWorld::onDestroy>(this);
}

void ControllerDynamicsWorld::update(const float delta) {
    dynamicsWorld->stepSimulation(delta, 10);
}

void ControllerDynamicsWorld::recalculate(VulkanRenderer& vulkan) {
}

void ControllerDynamicsWorld::onConstruct(entt::registry& r, entt::entity handle) {
}

void ControllerDynamicsWorld::onUpdate(entt::registry& r, entt::entity handle) {
    auto& component = reg.get<ComponentRigidBody>(handle);
    auto& transform = reg.get<ComponentTransform>(handle);
    component.update(transform);

    auto rigidBody = component.getRigidBody();
    if (rigidBody) {
        dynamicsWorld->addRigidBody(rigidBody);
    } else {
        logger.warn("ComponentRigidBody created with no rigid body attached");
    }

    logger.warn("Added rigid body");
}

void ControllerDynamicsWorld::onDestroy(entt::registry& r, entt::entity handle) {
    auto& component = reg.get<ComponentRigidBody>(handle);
    auto rigidBody = component.getRigidBody();
    if (rigidBody) {
        dynamicsWorld->removeRigidBody(rigidBody);
    }
}
