#include "ControllerGrid.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerGrid::ControllerGrid(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld,
                               VoxelShapeCache* voxelShapeCache) :
    scene{scene}, reg{reg}, dynamicsWorld{dynamicsWorld}, voxelShapeCache{voxelShapeCache} {

    reg.on_construct<ComponentGrid>().connect<&ControllerGrid::onConstruct>(this);
    reg.on_update<ComponentGrid>().connect<&ControllerGrid::onUpdate>(this);
    reg.on_destroy<ComponentGrid>().connect<&ControllerGrid::onDestroy>(this);
}

ControllerGrid::~ControllerGrid() {
    reg.on_construct<ComponentGrid>().disconnect<&ControllerGrid::onConstruct>(this);
    reg.on_update<ComponentGrid>().disconnect<&ControllerGrid::onUpdate>(this);
    reg.on_destroy<ComponentGrid>().disconnect<&ControllerGrid::onDestroy>(this);
}

void ControllerGrid::update(const float delta) {
    (void)delta;
}

void ControllerGrid::recalculate(VulkanRenderer& vulkan) {
    if (!voxelShapeCache) {
        EXCEPTION("Can not recalculate grid, voxel shape grid is null");
    }

    for (auto&& [_, grid] : reg.view<ComponentGrid>().each()) {
        grid.recalculate(vulkan, *voxelShapeCache);
    }

    particlesBatch.count = 0;

    for (auto&& [_, transform, grid, shipControl] :
         reg.view<ComponentTransform, ComponentGrid, ComponentShipControl>().each()) {

        auto strength = map(shipControl.getForwardVelocity(), 0.0f, shipControl.getForwardVelocityMax(), 0.0f, 3.0f);
        auto alpha =
            map(shipControl.getForwardVelocity(), 0.0f, shipControl.getForwardVelocityMax() / 5.0f, 0.0f, 1.0f);
        strength = glm::clamp(strength, 0.0f, 2.0f);
        alpha = glm::clamp(alpha, 0.0f, 1.0f);

        auto model = transform.getAbsoluteInterpolatedTransform();

        for (const auto& thruster : grid.getThrusters()) {
            if (!thruster.particles) {
                continue;
            }

            ComponentParticles::ParticlesBatchUniform batch{};
            batch.modelMatrix = model * thruster.mat;
            batch.timeDelta = vulkan.getRenderTime();
            batch.strength = strength;
            batch.alpha = alpha;
            addThrustParticles(vulkan, batch, thruster.particles);
        }
    }
}

void ControllerGrid::addOrUpdate(entt::entity handle, ComponentGrid& component) {
    auto* transform = reg.try_get<ComponentTransform>(handle);
    auto* rigidBody = reg.try_get<ComponentRigidBody>(handle);
    if (rigidBody && transform) {
        auto shape = component.createCollisionShape();
        if (shape) {
            transform->setScene(scene);
            rigidBody->setShape(*transform, std::move(shape));
            transform->setRigidBody(*rigidBody);
        }
    }
}

void ControllerGrid::onConstruct(entt::registry& r, entt::entity handle) {
    (void)r;
    (void)handle;
}

void ControllerGrid::onUpdate(entt::registry& r, entt::entity handle) {
    (void)r;
    auto& component = reg.get<ComponentGrid>(handle);
    addOrUpdate(handle, component);
}

void ControllerGrid::onDestroy(entt::registry& r, entt::entity handle) {
    (void)r;
    (void)handle;
}

void ControllerGrid::addThrustParticles(VulkanRenderer& vulkan,
                                        const ComponentParticles::ParticlesBatchUniform& uniform,
                                        const ParticlesTypePtr& particlesType) {
    if (!particlesBatch.uniforms) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(ComponentParticles::ParticlesBatchUniform) * particlesBatchSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        particlesBatch.uniforms = VulkanDoubleBuffer{vulkan, bufferInfo};

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                             VK_SHADER_STAGE_COMPUTE_BIT;

        particlesBatch.descriptorPool = VulkanDescriptorPool{vulkan, {&binding, 1}, MAX_FRAMES_IN_FLIGHT};
        particlesBatch.descriptorSetLayout = VulkanDescriptorSetLayout{vulkan, {&binding, 1}};

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            particlesBatch.descriptorSetsBatch[i] =
                particlesBatch.descriptorPool.createDescriptorSet(particlesBatch.descriptorSetLayout);
            particlesBatch.descriptorSetsBatch[i].bindUniform(
                0, particlesBatch.uniforms.getBuffers()[i], true, sizeof(ComponentParticles::ParticlesBatchUniform));
        }
    }

    if (particlesBatch.count >= particlesBatchSize) {
        return;
    }

    auto* dst = reinterpret_cast<ComponentParticles::ParticlesBatchUniform*>(
        particlesBatch.uniforms.getCurrentBuffer().getMappedPtr());

    const auto index = particlesBatch.count++;
    particlesBatch.descriptorSetsTypes[index] = &particlesType->getDescriptorSet();
    particlesBatch.descriptorSetsIndexes[index] = static_cast<uint32_t>(particlesType->getIndex());
    particlesBatch.counts[index] = particlesType->getCount();
    std::memcpy(&dst[index], &uniform, sizeof(uniform));
}
