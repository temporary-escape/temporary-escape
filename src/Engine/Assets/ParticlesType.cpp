#include "ParticlesType.hpp"
#include "AssetsManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ParticlesType::ParticlesType(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void ParticlesType::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)audio;

    try {
        Xml::fromFile(this->path, definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load particles: '{}'", getName());
    }
}

void ParticlesType::allocateUniforms(AssetsManager& assetsManager) {
    const auto [idx, uniform] = assetsManager.addParticleType();

    index = idx;

    uniform->startColor = definition.color.start;
    uniform->endColor = definition.color.end;
    uniform->duration = definition.duration;
    uniform->direction = definition.direction;
    uniform->count = definition.count;
    uniform->startSpawn = definition.spawn.start;
    uniform->endSpawn = definition.spawn.end;
    uniform->startSize = definition.size.start;
    uniform->endSize = definition.size.end;
}

ParticlesTypePtr ParticlesType::from(const std::string& name) {
    return AssetsManager::getInstance().getParticlesTypes().find(name);
}
