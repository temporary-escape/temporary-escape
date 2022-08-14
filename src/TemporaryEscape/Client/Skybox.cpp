#include "Skybox.hpp"

using namespace Engine;

Skybox::Skybox(VulkanDevice& vulkan, const Color4& color) {
    static const Vector2i size = {8, 8};
    std::unique_ptr<uint8_t[]> pixels(new uint8_t[size.x * size.y * 4]);

    for (size_t i = 0; i < size.x * size.y * 4; i += 4) {
        pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
        pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
        pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
        pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
    }

    auto desc = VulkanTexture::Descriptor{};
    desc.size = size;
    desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
    desc.levels = 1;
    desc.layers = 6;
    desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    desc.viewType = VulkanTexture::ViewType::VK_IMAGE_VIEW_TYPE_CUBE;
    desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    texture = vulkan.createTexture(desc);

    for (auto i = 0; i < 6; i++) {
        texture.subData(0, {0, 0}, i, size, pixels.get());
    }
}