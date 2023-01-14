#include "shader.hpp"
#include "../utils/exceptions.hpp"

#define CMP "ShaderModules"

using namespace Engine;

static const std::unordered_map<std::string, VkShaderStageFlagBits> extensionStageMap = {
    {".vert", VK_SHADER_STAGE_VERTEX_BIT},
    {".frag", VK_SHADER_STAGE_FRAGMENT_BIT},
    {".geom", VK_SHADER_STAGE_GEOMETRY_BIT},
    {".comp", VK_SHADER_STAGE_COMPUTE_BIT},
};

ShaderModules::ShaderModules(const Config& config, VulkanRenderer& vulkan) {
    for (const auto& entry : Fs::directory_iterator(config.shadersPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto ext = entry.path().extension().string();
        const auto stage = extensionStageMap.find(ext);
        if (stage == extensionStageMap.end()) {
            EXCEPTION("Unknown shader stage: '{}'", entry.path());
        }

        loadQueue.emplace_back([this, &vulkan, entry, stage]() {
            auto shader = vulkan.createShaderModule(entry.path(), stage->second);
            auto it = modules.insert(std::make_pair(entry.path().filename().string(), std::move(shader))).first;
            Log::d(CMP, "Added shader with name: '{}' flags: {}", it->first, stage->second);
        });
    }

    if (loadQueue.empty()) {
        EXCEPTION("No shaders found in: '{}'", config.shadersPath);
    }
}

VulkanShaderModule& ShaderModules::findByName(const std::string& name) {
    auto it = modules.find(name);
    if (it == modules.end()) {
        EXCEPTION("No such shader module named: '{}'", name);
    }
    return it->second;
}
