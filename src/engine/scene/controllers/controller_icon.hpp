#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerIcon : public Controller {
public:
    using Point = ComponentPointCloud::Point;

    explicit ControllerIcon(entt::registry& reg);
    ~ControllerIcon() override;

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    const std::unordered_map<Image*, std::unique_ptr<VulkanDoubleBuffer>>& getVbos() const {
        return vbos;
    }

    const std::unordered_map<Image*, size_t>& getCounts() const {
        return counts;
    }

private:
    VulkanDoubleBuffer& getBufferFor(VulkanRenderer& vulkan, const ImagePtr& image);
    std::unique_ptr<VulkanDoubleBuffer> createVbo(VulkanRenderer& vulkan);
    entt::registry& reg;

    std::unordered_map<Image*, std::unique_ptr<VulkanDoubleBuffer>> vbos;
    std::unordered_map<Image*, size_t> counts;
};
} // namespace Engine
