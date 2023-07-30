#include "render_pass_shadows.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassShadows::Internal::Internal(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                      const Vector2i& viewport, VulkanTexture& depthTexture,
                                      VulkanImageView& depthImageView, const size_t index) :
    RenderPass{vulkan, viewport}, subpassShadows{vulkan, resources, assetsManager, index} {

    addAttachment(depthTexture,
                  depthImageView,
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_CLEAR,
                  VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addSubpass(subpassShadows);
    init();
}

void RenderPassShadows::Internal::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Shadows].depthStencil = {1.0f, 0};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassShadows.render(vkb, scene);

    vkb.endRenderPass();
}

RenderPassShadows::RenderPassShadows(Engine::VulkanRenderer& vulkan, Engine::RenderResources& resources,
                                     Engine::AssetsManager& assetsManager, const Engine::Vector2i& viewport) {
    const auto format = vulkan.findDepthFormat();

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = format;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(viewport.x), static_cast<uint32_t>(viewport.y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 4;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = format;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 4;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = 0.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    shadowMapArray = vulkan.createTexture(textureInfo);

    for (size_t i = 0; i < internal.size(); i++) {
        VulkanImageView::CreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.format = textureInfo.image.format;
        viewInfo.subresourceRange = {};
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = i;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.image = shadowMapArray.getHandle();

        shadowMapViews.at(i) = vulkan.createImageView(viewInfo);
    }

    for (size_t i = 0; i < internal.size(); i++) {
        internal.at(i) = std::make_unique<Internal>(
            vulkan, resources, assetsManager, viewport, shadowMapArray, shadowMapViews.at(i), i);
    }
}

void RenderPassShadows::render(Engine::VulkanCommandBuffer& vkb, const Engine::Vector2i& viewport,
                               Engine::Scene& scene) {
    for (const auto& ptr : internal) {
        ptr->render(vkb, viewport, scene);
    }
}
