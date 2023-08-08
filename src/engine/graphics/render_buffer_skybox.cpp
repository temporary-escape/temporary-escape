#include "render_buffer_skybox.hpp"

using namespace Engine;

RenderBufferSkybox::RenderBufferSkybox(const RenderOptions& options, VulkanRenderer& vulkan) : RenderBuffer{vulkan} {
    setAttachments(5);

    { // Heightmap
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Color, texture, view);
    }
}
