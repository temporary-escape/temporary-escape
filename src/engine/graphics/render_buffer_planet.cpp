#include "render_buffer_planet.hpp"

using namespace Engine;

RenderBufferPlanet::RenderBufferPlanet(const Vector2i& viewport, VulkanRenderer& vulkan) : RenderBuffer{vulkan} {
    setAttachments(5);

    { // Heightmap
        TextureInfo info{};
        info.size = viewport;
        info.format = VK_FORMAT_R8_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Heightmap, texture, view);
    }

    { // Moisture
        TextureInfo info{};
        info.size = viewport;
        info.format = VK_FORMAT_R8_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Moisture, texture, view);
    }

    { // Color
        TextureInfo info{};
        info.size = viewport;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Color, texture, view);
    }

    { // Metallic Roughness
        TextureInfo info{};
        info.size = viewport;
        info.format = VK_FORMAT_R8G8_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::MetallicRoughness, texture, view);
    }

    { // Normal
        TextureInfo info{};
        info.size = viewport;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Normal, texture, view);
    }
}
