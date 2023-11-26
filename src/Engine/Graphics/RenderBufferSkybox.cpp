#include "RenderBufferSkybox.hpp"

using namespace Engine;

RenderBufferSkybox::RenderBufferSkybox(const Config& config, VulkanRenderer& vulkan) : RenderBuffer{vulkan} {
    setAttachments(3);

    { // Color
        TextureInfo info{};
        info.size = {config.graphics.skyboxSize, config.graphics.skyboxSize};
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Color, texture, view);
    }

    { // Irradiance
        TextureInfo info{};
        info.size = {config.graphics.skyboxIrradianceSize * 2, config.graphics.skyboxIrradianceSize};
        info.format = VK_FORMAT_R16G16B16A16_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Irradiance, texture, view);
    }

    { // Prefilter
        TextureInfo info{};
        info.size = {config.graphics.skyboxPrefilterSize * 2, config.graphics.skyboxPrefilterSize};
        info.format = VK_FORMAT_R16G16B16A16_UNORM;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        createAttachment(Attachment::Prefilter, texture, view);
    }
}
