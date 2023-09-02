#include "controller_turret.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerTurret::ControllerTurret(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld) :
    reg{reg}, dynamicsWorld{dynamicsWorld} {
    reg.on_destroy<ComponentTransform>().disconnect<&ControllerTurret::onDestroyTransform>(this);
}

ControllerTurret::~ControllerTurret() {
    reg.on_destroy<ComponentTransform>().disconnect<&ControllerTurret::onDestroyTransform>(this);
}

void ControllerTurret::update(const float delta) {
    for (auto& bullet : bullets) {
        if (bullet.lifetime < 0.0f) {
            continue;
        }

        bullet.lifetime -= delta;

        const auto advance = bullet.direction * bullet.speed * delta;

        ControllerDynamicsWorld::RayCastResult rayCastResult;
        dynamicsWorld.rayCast(bullet.origin, bullet.origin + advance, rayCastResult);

        if (rayCastResult) {
            bullet.lifetime = 0.0f;
            bullet.size = 0.0f;
            continue;
        }

        bullet.origin += advance;

        if (bullet.lifetime < 0.0f) {
            bullet.size = 0.0f;
        }
    }

    const auto& entities =
        reg.view<ComponentTransform, ComponentTurret, ComponentModelSkinned>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, turret, model] : entities) {
        if (turret.isActive()) {
            turret.update(delta, transform, model);

            if (turret.shouldShoot()) {
                turret.resetShoot();
                auto& bullet = allocateBullet();
                bullet.origin = transform.getAbsolutePosition();
                bullet.lifetime = 10.0f;
                bullet.direction = turret.getTargetDirection();
                bullet.size = 10.0f;
                bullet.speed = 500.0f;
            }
        }
    }
}

void ControllerTurret::recalculate(VulkanRenderer& vulkan) {
    if (vboBullets.getSize() != bullets.capacity() * sizeof(ComponentTurret::BulletInstance)) {
        logger.info("Recreating bullets buffer count: {}", bullets.capacity());

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bullets.capacity() * sizeof(ComponentTurret::BulletInstance);
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vulkan.dispose(std::move(vboBullets));
        vboBullets = vulkan.createDoubleBuffer(bufferInfo);
    }

    if (vboBullets) {
        vboBullets.subDataLocal(bullets.data(), 0, vboBullets.getSize());
    }
}

/*void ControllerTurret::receiveBullets(const msgpack::object& obj) {
    if (obj.type != msgpack::type::ARRAY) {
        EXCEPTION("Bullet update message is not an array");
    }

    const auto& arr = obj.via.array;

    for (size_t i = 0; i < arr.size; i++) {
    }
}*/

void ControllerTurret::onDestroyTransform(entt::registry& r, entt::entity handle) {
    (void)r;

    const auto& transform = reg.get<ComponentTransform>(handle);
    for (const auto&& [_, turret] : reg.view<ComponentTurret>().each()) {
        if (turret.getTarget() == &transform) {
            turret.setTarget(nullptr);
        }
    }
}

ComponentTurret::BulletInstance& ControllerTurret::allocateBullet() {
    for (auto& bullet : bullets) {
        if (bullet.lifetime <= 0.0f) {
            return bullet;
        }
    }
    return bullets.emplace_back();
}
