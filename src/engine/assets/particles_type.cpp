#include "particles_type.hpp"
#include "../server/lua.hpp"
#include "assets_manager.hpp"
#include <sol/sol.hpp>

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

    // Do not load unless Vulkan is present (client mode)
    if (!vulkan) {
        return;
    }

    Uniform uniform{};
    uniform.startColor = definition.color.start;
    uniform.endColor = definition.color.end;
    uniform.duration = definition.duration;
    uniform.direction = definition.direction;
    uniform.count = definition.count;
    uniform.startSpawn = definition.spawn.start;
    uniform.endSpawn = definition.spawn.end;
    uniform.startSize = definition.size.start;
    uniform.endSize = definition.size.end;

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Uniform);
    bufferInfo.usage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    ubo = vulkan->createBuffer(bufferInfo);
    vulkan->copyDataToBuffer(ubo, &uniform, sizeof(Uniform));
}

ParticlesTypePtr ParticlesType::from(const std::string& name) {
    return AssetsManager::getInstance().getParticlesTypes().find(name);
}

void ParticlesType::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ParticlesType>("ParticlesType");
    cls["name"] = sol::property(&ParticlesType::getName);
}

static_assert(offsetof(ParticlesType::Uniform, startSize) == 96);
static_assert(offsetof(ParticlesType::Uniform, endSize) == 104);
