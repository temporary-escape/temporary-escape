#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineShadowsModel : public RenderPipeline {
public:
    explicit RenderPipelineShadowsModel(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setEntityColor(const Color4& value);
};
} // namespace Engine
