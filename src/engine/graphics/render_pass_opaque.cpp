#include "render_pass_opaque.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

RenderPassOpaque::RenderPassOpaque(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                                   VoxelShapeCache& voxelShapeCache, const VulkanTexture& depth) :
    RenderPass{vulkan, viewport}, subpassOpaque{vulkan, registry, voxelShapeCache} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    // Depth
    addAttachment(depth,
                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_CLEAR,
                  VK_ATTACHMENT_LOAD_OP_LOAD);

    // AlbedoAmbient
    addAttachment({VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // EmissiveRoughness
    addAttachment({VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // NormalMetallic
    addAttachment({VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    addSubpass(subpassOpaque);
    init();
}

void RenderPassOpaque::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Depth].depthStencil = {1.0f, 0};
    renderPassInfo.clearValues[Attachments::AlbedoAmbient].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    renderPassInfo.clearValues[Attachments::EmissiveRoughness].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    renderPassInfo.clearValues[Attachments::NormalMetallic].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassOpaque.render(vkb, scene);

    vkb.endRenderPass();
}
