#include "PlanetType.hpp"
#include "AssetsManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

PlanetType::PlanetType(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void PlanetType::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)assetsManager;
    (void)audio;

    try {
        Xml::fromFile(this->path, definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load planet type: '{}'", getName());
    }

    // Do not load unless Vulkan is present (client mode)
    if (!vulkan) {
        return;
    }

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Definition::Amotsphere);
    bufferInfo.usage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    ubo = vulkan->createBuffer(bufferInfo);
    vulkan->copyDataToBuffer(ubo, &definition.atmosphere, sizeof(Definition::Amotsphere));
}

PlanetTypePtr PlanetType::from(const std::string& name) {
    return AssetsManager::getInstance().getPlanetTypes().find(name);
}
