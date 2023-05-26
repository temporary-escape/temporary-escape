#pragma once

#include "render_pass.hpp"
#include "render_subpass_bloom_extract.hpp"
#include "render_subpass_blur.hpp"
#include "render_subpass_forward.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque;
class ENGINE_API RenderPassLighting;

class ENGINE_API RenderPassBloom {
public:
    class Downsample : public RenderPass {
    public:
        enum Attachments {
            Color = 0,
        };

        explicit Downsample(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                            const VulkanTexture& forward);
        virtual ~Downsample() = default;

        void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

    private:
        const VulkanTexture& forward;
    };

    class Extract : public RenderPass {
    public:
        enum Attachments {
            Color = 0,
        };

        explicit Extract(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                         const VulkanTexture& forward);
        virtual ~Extract() = default;

        void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

    private:
        RenderSubpassBloomExtract subpassBloomExtract;
    };

    class Blur : public RenderPass {
    public:
        enum Attachments {
            Color = 0,
        };

        explicit Blur(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                      const VulkanTexture& dst, const VulkanTexture& color, bool vertical);
        virtual ~Blur() = default;

        void reset();
        void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

    private:
        RenderSubpassBlur subpassBloomBlur;
    };

    explicit RenderPassBloom(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                             const VulkanTexture& forward);
    virtual ~RenderPassBloom() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

    const VulkanTexture& getBluredTexture() const {
        return extract.getTexture(Extract::Attachments::Color);
    }

private:
    Downsample downsample;
    Extract extract;
    Blur blurH;
    Blur blurV;
};
} // namespace Engine
