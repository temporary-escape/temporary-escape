#include "Sound.hpp"
#include "../File/OggFileReader.hpp"
#include "AssetsManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Sound::Sound(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Sound::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    // Do not load unless Audio is present (client mode)
    if (!audio) {
        return;
    }

    try {
        OggFileReader file{path};
        const auto freq = file.getFrequency();
        const auto format = file.getFormat();
        const auto data = file.readData();
        buffer = audio->createBuffer(data.data(), data.size(), format, freq);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load sound: '{}'", getName());
    }
}

SoundPtr Sound::from(const std::string& name) {
    return AssetsManager::getInstance().getSounds().find(name);
}
