#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelinePlanetMoisture : public RenderPipeline {
public:
    explicit RenderPipelinePlanetMoisture(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setIndex(int value);
    void setSeed(float value);
    void setResolution(float value);
    void setRes1(float value);
    void setRes2(float value);
    void setResMix(float value);
    void setMixScale(float value);
    void setDoesRidged(float value);
};
} // namespace Engine
