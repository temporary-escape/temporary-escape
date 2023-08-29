#include "controller_model_skinned.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerModelSkinned::ControllerModelSkinned(entt::registry& reg) : reg{reg} {
    reg.on_construct<ComponentModelSkinned>().connect<&ControllerModelSkinned::onConstruct>(this);
    reg.on_update<ComponentModelSkinned>().connect<&ControllerModelSkinned::onUpdate>(this);
    reg.on_destroy<ComponentModelSkinned>().connect<&ControllerModelSkinned::onDestroy>(this);

    VulkanArrayBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.stride = sizeof(ComponentModelSkinned::Armature);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    buffer = VulkanArrayBuffer{bufferInfo};
}

ControllerModelSkinned::~ControllerModelSkinned() {
    reg.on_construct<ComponentModelSkinned>().disconnect<&ControllerModelSkinned::onConstruct>(this);
    reg.on_update<ComponentModelSkinned>().disconnect<&ControllerModelSkinned::onUpdate>(this);
    reg.on_destroy<ComponentModelSkinned>().disconnect<&ControllerModelSkinned::onDestroy>(this);
}

void ControllerModelSkinned::update(const float delta) {
}

void ControllerModelSkinned::recalculate(VulkanRenderer& vulkan) {
    /*armatures.clear();
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
        if (ubo) {
            vulkan.dispose(std::move(ubo));
            ubo = VulkanDoubleBuffer{};
        }
        return;
    }

    if (!ubo || ubo.getSize() < bufferSize) {
        if (ubo) {
            vulkan.dispose(std::move(ubo));
            ubo = VulkanDoubleBuffer{};
        }

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        logger.debug("Recreating armature buffer with {} items", armatures.size());
        ubo = vulkan.createDoubleBuffer(bufferInfo);
    }

    ubo.subDataLocal(armatures.data(), 0, bufferSize);*/

    buffer.recalculate(vulkan);
}

void ControllerModelSkinned::addOrUpdate(entt::entity handle, ComponentModelSkinned& component) {
    const auto& data = component.getCache();
    if (data.count == 0) {
        buffer.remove(static_cast<uint64_t>(handle));
        return;
    }

    auto* raw = buffer.insert(static_cast<uint64_t>(handle));
    const auto offset = buffer.offsetOf(raw);
    auto* armature = reinterpret_cast<ComponentModelSkinned::Armature*>(raw);

    armature->count = static_cast<int>(data.count);

    /*Matrix4 parentTransform{1.0f};
    for (size_t i = 0; i < armature->count; i++) {
        const auto localTransform = parentTransform * data.adjustmentsMat[i] * data.jointsLocalMat[i];
        armature->joints[i] = localTransform * data.inverseBindMat[i];
        parentTransform = localTransform;
    }*/

    Matrix4 parentJointMat{1.0f};
    std::array<Matrix4, Model::maxJoints> jointsMat{};
    for (size_t i = 0; i < armature->count; i++) {
        jointsMat[i] = parentJointMat * data.jointsLocalMat[i];
        parentJointMat = jointsMat[i] * data.adjustmentsMat[i];
    }

    for (size_t i = 0; i < armature->count; i++) {
        armature->joints[i] = jointsMat[i] * data.adjustmentsMat[i] * data.inverseBindMat[i];
    }

    component.setUboOffset(offset);
}

void ControllerModelSkinned::remove(entt::entity handle) {
    buffer.remove(static_cast<uint64_t>(handle));
}

void ControllerModelSkinned::onConstruct(entt::registry& r, const entt::entity handle) {
    (void)r;
    auto& component = reg.get<ComponentModelSkinned>(handle);
    addOrUpdate(handle, component);
}

void ControllerModelSkinned::onUpdate(entt::registry& r, const entt::entity handle) {
    (void)r;
    auto& component = reg.get<ComponentModelSkinned>(handle);
    addOrUpdate(handle, component);
}

void ControllerModelSkinned::onDestroy(entt::registry& r, const entt::entity handle) {
    (void)r;
    remove(handle);
}
