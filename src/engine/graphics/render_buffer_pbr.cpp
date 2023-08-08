#include "render_buffer_pbr.hpp"

using namespace Engine;

RenderBufferPbr::RenderBufferPbr(const RenderOptions& options, VulkanRenderer& vulkan) : RenderBuffer{vulkan} {
    setAttachments(2);

    { // Depth
        TextureInfo info{};
        info.size = options.viewport;
        info.format = vulkan.findDepthFormat();
        info.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
        createAttachment(Attachment::Depth, texture, view);
    }

    { // Forward
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::Forward, texture, view);
    }
}
