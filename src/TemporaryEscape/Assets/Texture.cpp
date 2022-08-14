#include "Texture.hpp"
#include "../Utils/PngImporter.hpp"
#include "Registry.hpp"

#define CMP "Texture"

using namespace Engine;

Texture::Texture(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Texture::load(Registry& registry, VulkanDevice& vulkan) {
    (void)registry;

    Options options{};
    const auto optionsPath = path.parent_path() / (path.stem().string() + std::string(".yml"));

    if (Fs::exists(optionsPath)) {
        try {
            options.fromYaml(optionsPath);
        } catch (...) {
            EXCEPTION_NESTED("Failed to load texture options from: '{}'", optionsPath.string());
        }
    }

    try {
        PngImporter image(path);

        VulkanTexture::Descriptor desc{};
        desc.size = image.getSize();

        switch (image.getPixelType()) {
        case ImageImporter::PixelType::Rgb8u: {
            desc.format = VulkanTexture::Format::VK_FORMAT_R8G8B8_UNORM;
            break;
        }
        case ImageImporter::PixelType::Rgba8u: {
            desc.format = VulkanTexture::Format::VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        case ImageImporter::PixelType::Rgb16u: {
            desc.format = VulkanTexture::Format::VK_FORMAT_R16G16B16_UNORM;
            break;
        }
        case ImageImporter::PixelType::Rgba16u: {
            desc.format = VulkanTexture::Format::VK_FORMAT_R16G16B16A16_UNORM;
            break;
        }
        default: {
            EXCEPTION("Unsupported pixel format");
        }
        }

        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.levels = 1;

        texture = vulkan.createTexture(desc);
        texture.subData(0, {0, 0}, image.getSize(), image.getData());

        // TODO: Filtering and wrapping

    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

TexturePtr Texture::from(const std::string& name) {
    return Registry::getInstance().getTextures().find(name);
}
