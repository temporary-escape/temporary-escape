#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerIcon : public Controller {
public:
    using Point = ComponentIcon::Point;
    using Buffers = std::unordered_map<const VulkanTexture*, VulkanArrayBuffer>;

    explicit ControllerIcon(Scene& scene, entt::registry& reg);
    ~ControllerIcon() override;
    NON_COPYABLE(ControllerIcon);
    NON_MOVEABLE(ControllerIcon);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    const Buffers& getStaticBuffers() const {
        return staticBuffers;
    }
    const Buffers& getDynamicBuffers() const {
        return dynamicBuffers;
    }

private:
    void addOrUpdate(entt::entity handle, const ComponentTransform& transform, const ComponentIcon& icon);
    void remove(entt::entity handle, const ComponentIcon& icon);

    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    Scene& scene;
    entt::registry& reg;
    Buffers staticBuffers;
    Buffers dynamicBuffers;
};
} // namespace Engine
