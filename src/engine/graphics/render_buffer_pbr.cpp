#include "render_buffer_pbr.hpp"

using namespace Engine;

RenderBufferPbr::RenderBufferPbr(const RenderOptions& options, VulkanRenderer& vulkan) : RenderBuffer{vulkan} {
    setAttachments(12 + bloomMipMaps);

    { // Depth
        TextureInfo info{};
        info.size = options.viewport;
        info.format = vulkan.findDepthFormat();
        info.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
        createAttachment(Attachment::Depth, texture, view);
    }

    { // Forward
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::Forward, texture, view);
    }

    { // Albedo Ambient
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VkImageUsageFlagBits ::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::AlbedoAmbient, texture, view);
    }

    { // Emissive Roughness
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VkFormat::VK_FORMAT_R16G16B16A16_UNORM;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::EmissiveRoughness, texture, view);
    }

    { // Normal Metallic
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VkFormat::VK_FORMAT_R16G16B16A16_UNORM;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::NormalMetallic, texture, view);
    }

    { // Entity
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        info.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
        info.addressMode = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::Entity, texture, view);
    }

    if (options.shadowsSize) { // Shadows
        TextureInfo info{};
        info.size = {options.shadowsSize, options.shadowsSize};
        info.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        info.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
        info.format = vulkan.findDepthFormat();
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        info.layers = 4;
        auto& texture = createTexture(info);

        for (auto i = 0; i < 4; i++) {
            ViewInfo view{};
            view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            view.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
            view.arrayLayer = i;
            createAttachment(Attachment::ShadowL0 + i, texture, view);
        }
    }

    if (options.ssao) { // SSAO
        TextureInfo info{};
        info.size = options.viewport / 2;
        info.format = VkFormat::VK_FORMAT_R8_UNORM;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::SSAO, texture, view);
    }

    { // FXAA
        TextureInfo info{};
        info.size = options.viewport;
        info.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
        info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        auto& texture = createTexture(info);

        ViewInfo view{};
        view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        createAttachment(Attachment::FXAA, texture, view);
    }

    if (options.bloom) { // Bloom
        for (auto i = 0; i < bloomMipMaps; i++) {
            TextureInfo info{};
            info.size = options.viewport / (2 << i);
            info.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
            info.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            auto& texture = createTexture(info);

            ViewInfo view{};
            view.type = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
            createAttachment(Attachment::BloomL0 + i, texture, view);
        }
    }
}
