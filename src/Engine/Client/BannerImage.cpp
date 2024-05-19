#include "BannerImage.hpp"
#include "../File/Ktx2FileReader.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include <banner.ktx2.h>

using namespace Engine;

BannerImage::BannerImage(VulkanRenderer& vulkan) {
    Ktx2FileReader image{Embed::banner_ktx2};
    image.loadAsTexture(vulkan, texture);
}

void BannerImage::destroy() {
    texture.destroy();
}
