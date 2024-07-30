#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineModel : public RenderPipeline {
public:
    explicit RenderPipelineModel(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setEntityColor(const Color4& value);
};
} // namespace Engine
