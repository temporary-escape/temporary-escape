#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerStaticModel : public Controller {
public:
    explicit ControllerStaticModel(entt::registry& reg);
    ~ControllerStaticModel() override;
    NON_COPYABLE(ControllerStaticModel);
    NON_MOVEABLE(ControllerStaticModel);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    const std::unordered_map<Model*, VulkanArrayBuffer>& getBuffers() const {
        return buffers;
    }

private:
    void addOrUpdate(entt::entity handle, const ComponentTransform& transform, const ComponentModel& model);
    void remove(entt::entity handle, const ComponentModel& model);

    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    // ModelMatrixBuffer& getBufferFor(const ModelPtr& model, VulkanRenderer& vulkan);

    entt::registry& reg;
    std::unordered_map<Model*, VulkanArrayBuffer> buffers;
};
} // namespace Engine
