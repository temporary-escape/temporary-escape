#include "image.hpp"
#include "../utils/png_importer.hpp"
#include "registry.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

Image::Image(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

Image::Image(std::string name, const ImageAtlas::Allocation& allocation) :
    Asset{std::move(name)}, allocation{allocation} {
}

void Image::load(Registry& registry, VulkanRenderer& vulkan) {
    if (path.empty()) {
        return;
    }

    try {
        PngImporter image{path};

        if (image.getFormat() != VK_FORMAT_R8G8B8A8_UNORM) {
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
