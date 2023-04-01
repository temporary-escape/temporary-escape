#include "shader.hpp"
#include "registry.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

static const std::unordered_map<std::string, VkShaderStageFlagBits> extensionStageMap = {
    {".vert", VK_SHADER_STAGE_VERTEX_BIT},
    {".frag", VK_SHADER_STAGE_FRAGMENT_BIT},
    {".geom", VK_SHADER_STAGE_GEOMETRY_BIT},
    {".comp", VK_SHADER_STAGE_COMPUTE_BIT},
};

Shader::Shader(std::string name, Path path) : Asset{path.filename().string()}, path{std::move(path)} {
}

void Shader::load(Registry& registry, VulkanRenderer& vulkan) {
    try {
        const auto ext = path.extension().string();

        const auto stageIt = extensionStageMap.find(ext);
        if (stageIt == extensionStageMap.end()) {
            EXCEPTION("Unknown shader stage for extension: '{}'", ext);
        }

        stage = stageIt->second;
        shader = vulkan.createShaderModule(path, stage);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load shader: '{}'", getName());
    }
}

ShaderPtr Shader::from(const std::string& name) {
    return Registry::getInstance().getShaders().find(name);
}
