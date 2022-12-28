#pragma once

#include "vg_device.hpp"

namespace Engine {
class VgRenderer : public VgDevice, public VgCommandBuffer {
public:
    explicit VgRenderer(const Config& config);

    VgPipeline createPipeline(const VgPipeline::CreateInfo& createInfo);
    VgFramebuffer& getSwapChainFramebuffer();

    void beginRenderPass(const VgFramebuffer& framebuffer, const Vector2i& size);

protected:
    void render(const Vector2i& viewport, float deltaTime) override;
    void onSwapChainChanged() override;
    virtual void draw(const Vector2i& viewport, float deltaTime) = 0;

private:
    void createRenderPass();
    void createSwapChainFramebuffers();

    VgRenderPass renderPass;
    std::vector<VgFramebuffer> swapChainFramebuffers;
    Vector2i lastViewportSize;
};
} // namespace Engine
