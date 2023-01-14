#pragma once

#include "../assets/image.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPositionFeedback : public Component {
public:
    using Point = Vector3;
    using Output = Vector2;

    ComponentPositionFeedback() = default;
    virtual ~ComponentPositionFeedback() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPositionFeedback);

    void add(const Vector3& pos);
    void clear();
    void recalculate(VulkanRenderer& vulkan);

    const VulkanBuffer& getBufferInput() const {
        return sboInput;
    }

    const VulkanBuffer& getBufferOutput() const {
        return sboOutput.getCurrentBuffer();
    }

    size_t getCount() const {
        return points.size();
    }

private:
    std::vector<Point> points;
    VulkanBuffer sboInput;
    VulkanDoubleBuffer sboOutput;
};
} // namespace Engine
