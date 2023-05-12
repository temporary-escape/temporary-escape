#pragma once

#include "../assets/model.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentNebula : public Component {
public:
    struct Uniform {
        Vector4 color{};
        Vector4 offset{};
        float scale{0};
        float intensity{0};
        float falloff{0};
    };

    ComponentNebula();
    virtual ~ComponentNebula() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentNebula);

    void recalculate(VulkanRenderer& vulkan);

    const Color4& getColor() const {
        return uniform.color;
    }

    void setColor(const Color4& value) {
        uniform.color = value;
    }

    const Vector4& getOffset() const {
        return uniform.offset;
    }

    void setOffset(const Vector4& value) {
        uniform.offset = value;
    }

    float getScale() const {
        return uniform.scale;
    }

    void setScale(float value) {
        uniform.scale = value;
    }

    float getIntensity() const {
        return uniform.intensity;
    }

    void setIntensity(float value) {
        uniform.intensity = value;
    }

    float getFalloff() const {
        return uniform.falloff;
    }

    void setFalloff(float value) {
        uniform.falloff = value;
    }

    const VulkanBuffer& getUbo() const {
        return ubo;
    }

private:
    Uniform uniform;
    VulkanBuffer ubo;
};
} // namespace Engine
