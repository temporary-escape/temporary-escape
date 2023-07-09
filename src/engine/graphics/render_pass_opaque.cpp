#include "render_pass_opaque.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassOpaque::RenderPassOpaque(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                   const Vector2i& viewport, VoxelShapeCache& voxelShapeCache,
                                   const VulkanTexture& depth) :
    RenderPass{vulkan, viewport}, subpassOpaque{vulkan, resources, assetsManager, voxelShapeCache} {

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
    addAttachment({VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // NormalMetallic
    addAttachment({VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Entity
    addAttachment({VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    addSubpass(subpassOpaque);
    init();

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = 4; // 1 pixel RGBA
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    entityColorBuffer = vulkan.createDoubleBuffer(bufferInfo);
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
    renderPassInfo.clearValues[Attachments::Entity].color = {{1.0f, 1.0f, 1.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassOpaque.render(vkb, scene);

    vkb.endRenderPass();

    if (mousePos.x >= 0 && mousePos.x < viewport.x && mousePos.y >= 0 && mousePos.y < viewport.y) {
        std::array<VkBufferImageCopy, 1> regions{};
        regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions[0].imageSubresource.layerCount = 1;
        regions[0].imageExtent.width = 1;
        regions[0].imageExtent.height = 1;
        regions[0].imageExtent.depth = 1;
        regions[0].imageOffset.x = mousePos.x;
        regions[0].imageOffset.y = mousePos.y;
        regions[0].imageOffset.z = 0;

        vkb.copyImageToBuffer(getTexture(Attachments::Entity),
                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              entityColorBuffer.getCurrentBuffer(),
                              regions);
    }
}

uint32_t RenderPassOpaque::getMousePosEntity() {
    const auto* src = static_cast<const unsigned char*>(entityColorBuffer.getCurrentBuffer().getMappedPtr());
    uint32_t id{0};
    id |= static_cast<uint32_t>(src[0]);
    id |= static_cast<uint32_t>(src[1]) << 8;
    id |= static_cast<uint32_t>(src[2]) << 16;
    id |= static_cast<uint32_t>(src[3]) << 24;
    return id;
}
