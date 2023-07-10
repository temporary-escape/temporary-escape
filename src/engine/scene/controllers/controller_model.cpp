#include "controller_model.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerModel::ControllerModel(entt::registry& reg) : reg{reg} {
    reg.on_construct<ComponentTransform>().connect<&ControllerModel::onConstruct>(this);
    reg.on_destroy<ComponentTransform>().connect<&ControllerModel::onDestroy>(this);
}

ControllerModel::~ControllerModel() {
    reg.on_construct<ComponentTransform>().connect<&ControllerModel::onConstruct>(this);
    reg.on_destroy<ComponentTransform>().connect<&ControllerModel::onDestroy>(this);
}

void ControllerModel::update(const float delta) {
}

void ControllerModel::recalculate(VulkanRenderer& vulkan) {
    /*const auto view = reg.view<ComponentTransform, ComponentModel>(entt::exclude<TagDisabled>);

    for (auto&& [_, buffer] : buffers) {
        buffer.count = 0;
    }

    for (auto&& [entity, transform, model] : view.each()) {
        if (!model.isStatic()) {
            continue;
        }

        auto& buffer = getBufferFor(model.getModel(), vulkan);
        auto* dstModels = reinterpret_cast<Matrix4*>(buffer.vboModels.getCurrentBuffer().getMappedPtr());
        auto* dstEntityColor = reinterpret_cast<Vector4*>(buffer.vboEntityColors.getCurrentBuffer().getMappedPtr());

        if (!dstModels || !dstEntityColor) {
            EXCEPTION("Mapped buffer is null");
        }

        if (buffer.count + 1 < buffer.capacity) {
            dstModels[buffer.count] = transform.getAbsoluteTransform();
            dstEntityColor[buffer.count] = entityColor(entity);
            ++buffer.count;
        } else {
            buffer.expand = true;
        }
    }*/
}

/*ControllerModel::ModelMatrixBuffer& ControllerModel::getBufferFor(const ModelPtr& model, VulkanRenderer& vulkan) {
    auto& buffer = buffers[model.get()];

    if (buffer.expand) {
        buffer.capacity += 1024;

        logger.info("Expanded instanced model buffers for: {} capacity: {}", model->getName(), buffer.capacity);

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Matrix4) * buffer.capacity;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        buffer.vboModels = vulkan.createDoubleBuffer(bufferInfo);

        bufferInfo.size = sizeof(Vector4) * buffer.capacity;
        buffer.vboEntityColors = vulkan.createDoubleBuffer(bufferInfo);

        buffer.expand = false;
    }

    return buffer;
}*/

void ControllerModel::onConstruct(entt::registry& r, const entt::entity handle) {
    (void)r;
}

void ControllerModel::onDestroy(entt::registry& r, const entt::entity handle) {
    (void)r;
}
