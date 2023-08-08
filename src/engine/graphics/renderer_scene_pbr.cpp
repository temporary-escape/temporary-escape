#include "renderer_scene_pbr.hpp"
#include "../scene/scene.hpp"
#include "../utils/exceptions.hpp"
#include "passes/render_pass_bloom_downsample.hpp"
#include "passes/render_pass_bloom_upsample.hpp"
#include "passes/render_pass_forward.hpp"
#include "passes/render_pass_fxaa.hpp"
#include "passes/render_pass_hdr_mapping.hpp"
#include "passes/render_pass_opaque.hpp"
#include "passes/render_pass_pbr.hpp"
#include "passes/render_pass_shadow.hpp"
#include "passes/render_pass_skybox.hpp"
#include "passes/render_pass_ssao.hpp"

using namespace Engine;

RendererScenePbr::RendererScenePbr(const RenderOptions& options, VulkanRenderer& vulkan, RenderResources& resources,
                                   AssetsManager& assetsManager) :
    Renderer{vulkan},
    vulkan{vulkan},
    resources{resources},
    renderBufferPbr{options, vulkan},
    pipelineBlit{vulkan, assetsManager} {

    try {
        pipelineBlit.create(vulkan.getRenderPass(), 0, {0});
    } catch (...) {
        EXCEPTION_NESTED("Failed to create pipeline: {}", pipelineBlit.getName());
    }

    try {
        addRenderPass(std::make_unique<RenderPassSkybox>(vulkan, renderBufferPbr, resources, assetsManager));
        addRenderPass(std::make_unique<RenderPassOpaque>(vulkan, renderBufferPbr, resources, assetsManager));
        for (auto i = 0; i < 4; i++) {
            addRenderPass(std::make_unique<RenderPassShadow>(vulkan, renderBufferPbr, resources, assetsManager, i));
        }
        addRenderPass(std::make_unique<RenderPassSSAO>(vulkan, renderBufferPbr, resources, assetsManager));
        addRenderPass(std::make_unique<RenderPassPbr>(vulkan, renderBufferPbr, resources, assetsManager));
        addRenderPass(std::make_unique<RenderPassForward>(vulkan, renderBufferPbr, resources, assetsManager));
        addRenderPass(std::make_unique<RenderPassFXAA>(vulkan, renderBufferPbr, resources, assetsManager));
        for (auto i = 0; i < RenderBufferPbr::bloomMipMaps; i++) {
            addRenderPass(
                std::make_unique<RenderPassBloomDownsample>(vulkan, renderBufferPbr, resources, assetsManager, i));
        }
        for (int i = RenderBufferPbr::bloomMipMaps - 2; i >= 0; i--) {
            addRenderPass(
                std::make_unique<RenderPassBloomUpsample>(vulkan, renderBufferPbr, resources, assetsManager, i));
        }
        addRenderPass(std::make_unique<RenderPassHDRMapping>(vulkan, renderBufferPbr, resources, assetsManager));
    } catch (...) {
        EXCEPTION_NESTED("Failed to setup render passes");
    }
    create();
}

void RendererScenePbr::render(VulkanCommandBuffer& vkb, Scene& scene) {
    const auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Failed to render scene error: no primary camera");
    }

    camera->recalculate(vulkan, getViewport());

    Renderer::render(vkb, scene);
}

void RendererScenePbr::transitionForBlit(VulkanCommandBuffer& vkb) {
    const auto& src = renderBufferPbr.getAttachmentTexture(RenderBufferPbr::Attachment::Forward);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = src.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = src.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = src.getLayerCount();
    barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask =
        VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

    vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        barrier);
}

void RendererScenePbr::blit(VulkanCommandBuffer& vkb) {
    const auto& src = renderBufferPbr.getAttachmentTexture(RenderBufferPbr::Attachment::Forward);
    pipelineBlit.getDescriptionPool().reset();
    pipelineBlit.bind(vkb);
    pipelineBlit.setTexture(src);
    pipelineBlit.flushDescriptors(vkb);
    pipelineBlit.renderMesh(vkb, resources.getMeshFullScreenQuad());
}

Vector2i RendererScenePbr::getViewport() const {
    const auto& texture = renderBufferPbr.getAttachmentTexture(RenderBufferPbr::Attachment::Forward);
    return {texture.getExtent().width, texture.getExtent().height};
}
