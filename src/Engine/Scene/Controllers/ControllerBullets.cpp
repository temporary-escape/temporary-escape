#include "ControllerBullets.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerBullets::ControllerBullets(Scene& scene, entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld) :
    scene{scene}, reg{reg}, dynamicsWorld{dynamicsWorld} {
}

ControllerBullets::~ControllerBullets() {
}

void ControllerBullets::update(const float delta) {
    for (auto& bullet : bullets) {
        if (bullet.lifetime <= 0.0f) {
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
}

void ControllerBullets::recalculate(VulkanRenderer& vulkan) {
    if (vbo.getSize() != bullets.capacity() * sizeof(ComponentTurret::BulletInstance)) {
        logger.info("Recreating bullets buffer count: {}", bullets.capacity());

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bullets.capacity() * sizeof(ComponentTurret::BulletInstance);
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vulkan.dispose(std::move(vbo));
        vbo = vulkan.createDoubleBuffer(bufferInfo);
    }

    if (vbo) {
        vbo.subDataLocal(bullets.data(), 0, vbo.getSize());
    }
}

ComponentTurret::BulletInstance& ControllerBullets::allocateBullet() {
    for (auto& bullet : bullets) {
        if (bullet.lifetime <= 0.0f) {
            return bullet;
        }
    }
    return bullets.emplace_back();
}
