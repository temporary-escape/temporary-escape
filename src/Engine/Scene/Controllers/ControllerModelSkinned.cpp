#include "ControllerModelSkinned.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerModelSkinned::ControllerModelSkinned(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
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
