#pragma once

#include "../vulkan/vulkan_renderer.hpp"
#include "render_options.hpp"

namespace Engine {
class ENGINE_API RenderBuffer {
public:
    explicit RenderBuffer(VulkanRenderer& vulkan);
    virtual ~RenderBuffer();
    NON_MOVEABLE(RenderBuffer);
    NON_COPYABLE(RenderBuffer);

    [[nodiscard]] const VulkanImageView& getAttachment(const uint32_t attachment) const {
        return attachments.at(attachment).view;
    }

    [[nodiscard]] const VulkanTexture& getAttachmentTexture(uint32_t attachment) const;

protected:
    struct TextureInfo {
        Vector2i size;
        VkFormat format{VK_FORMAT_UNDEFINED};
        VkImageType type{VK_IMAGE_TYPE_2D};
        VkImageViewType viewType{VK_IMAGE_VIEW_TYPE_2D};
        VkImageUsageFlags usage{0};
        uint32_t layers{1};
        VkImageAspectFlags aspectMask{VK_IMAGE_ASPECT_COLOR_BIT};
        VkSamplerAddressMode addressMode{VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        VkBorderColor borderColor{VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK};
    };

    struct ViewInfo {
        VkImageViewType type{VK_IMAGE_VIEW_TYPE_2D};
        VkImageAspectFlags aspectMask{VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT};
        uint32_t arrayLayer{0};
    };

    VulkanTexture& createTexture(const TextureInfo& info);
    void createAttachment(uint32_t attachment, VulkanTexture& texture, const ViewInfo& info);
    void setAttachments(size_t count) {
        attachments.resize(count);
    }

private:
    struct AttachmentData {
        VulkanTexture* texture{nullptr};
        VulkanImageView view{};
        /*VkImageLayout layout{VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED};
        VkAccessFlags access{VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
        VkPipelineStageFlags stage{VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT};*/
    };

    VulkanRenderer& vulkan;
    std::list<VulkanTexture> textures;
    std::vector<AttachmentData> attachments;
};
} // namespace Engine
