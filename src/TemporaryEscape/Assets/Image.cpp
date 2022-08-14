#include "Image.hpp"
#include "../Utils/PngImporter.hpp"
#include "Registry.hpp"

#define CMP "Texture"

using namespace Engine;

Image::Image(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Image::load(Registry& registry, VulkanDevice& vulkan) {
    (void)registry;

    try {
        PngImporter image(path);

        VulkanTexture::Descriptor desc{};
        desc.size = image.getSize();
        if (image.getPixelType() != ImageImporter::PixelType::Rgba8u) {
            EXCEPTION("Image must be of format RGBA 8bit");
        }

        allocation = registry.getImageAtlas().add(image.getSize(), image.getData());

    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

ImagePtr Image::from(const std::string& name) {
    return Registry::getInstance().getImages().find(name);
}
