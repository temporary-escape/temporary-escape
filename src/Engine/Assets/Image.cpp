#include "Image.hpp"
#include "../File/Ktx2FileReader.hpp"
#include "AssetsManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Image::Image(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

Image::Image(std::string name, const ImageAtlas::Allocation& allocation) :
    Asset{std::move(name)}, allocation{allocation} {
}

void Image::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)audio;
    (void)assetsManager;

    // Do not load unless Vulkan is present (client mode)
    if (!vulkan) {
        return;
    }

    if (path.empty()) {
        return;
    }

    try {
        Ktx2FileReader image{path};

        if (image.needsTranscoding()) {
            image.transcode(VulkanCompressionType::None, TextureCompressionTarget::RGBA);
        }

        image.readData();

        if (image.getFormat() != VK_FORMAT_R8G8B8A8_UNORM) {
            EXCEPTION("Image must be of format RGBA 8bit");
        }

        allocation = assetsManager.getImageAtlas().add(image.getSize(), image.getData(0, 0).pixels.data());

    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

ImagePtr Image::from(const std::string& name) {
    return AssetsManager::getInstance().getImages().find(name);
}
