#include "controller_model_skinned.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerModelSkinned::ControllerModelSkinned(entt::registry& reg) : reg{reg} {
}

ControllerModelSkinned::~ControllerModelSkinned() = default;

void ControllerModelSkinned::update(const float delta) {
}

void ControllerModelSkinned::recalculate(VulkanRenderer& vulkan) {
    armatures.clear();
    items.clear();

    auto view = reg.view<ComponentTransform, ComponentModelSkinned>(entt::exclude<TagDisabled>);
    for (auto&& [entity, transform, component] : view.each()) {
        const auto& model = component.getModel();
        if (!model || model->getNodes().empty()) {
            continue;
        }

        const auto& node = model->getNodes().front();
        if (!node.skin) {
            continue;
        }

        const auto offset = armatures.size() * sizeof(ComponentModelSkinned::Armature);

        if (armatures.capacity() <= armatures.size()) {
            armatures.reserve(armatures.capacity() + 64);
            items.reserve(armatures.capacity() + 64);
        }

        auto& armature = armatures.emplace_back();
        armature.count = static_cast<int>(node.skin.count);

        Matrix4 previous{1.0f};
        for (size_t i = 0; i < armature.count; i++) {
            armature.joints[i] = previous * node.skin.jointsLocalMat[i] * node.skin.inverseBindMat[i];
            previous = previous * node.skin.jointsLocalMat[i];
        }

        auto& item = items.emplace_back();
        item.transform = transform.getAbsoluteTransform();
        item.offset = offset;
        item.model = model.get();
        item.entity = entity;
    }

    const auto bufferSize = armatures.size() * sizeof(ComponentModelSkinned::Armature);
    if (!bufferSize) {
        return;
    }

    if (!ubo || ubo.getSize() != bufferSize) {
        if (ubo) {
            vulkan.dispose(std::move(ubo));
        }

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        logger.debug("Recreating armature UBO with {} items", armatures.size());
        ubo = vulkan.createDoubleBuffer(bufferInfo);
    }

    ubo.subDataLocal(armatures.data(), 0, bufferSize);
}
