#include "image.hpp"
#include "../utils/png_importer.hpp"
#include "registry.hpp"

#define CMP "Texture"

using namespace Engine;

Image::Image(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Image::load(Registry& registry, VulkanRenderer& vulkan) {
    (void)registry;

    try {
        PngImporter image{path};

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
