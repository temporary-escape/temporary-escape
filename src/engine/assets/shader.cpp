#include "shader.hpp"
#include "../vulkan/glsl_compiler.hpp"
#include "assets_manager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Shader::Shader(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Shader::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)assetsManager;
    (void)audio;

    // Do not load unless Vulkan is present (client mode)
    if (!vulkan) {
        return;
    }

    try {
        stage = getGLSLFileFlags(path.filename().string());
        shader = vulkan->createShaderModule(path, stage);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load shader: '{}'", getName());
    }
}

ShaderPtr Shader::from(const std::string& name) {
    return AssetsManager::getInstance().getShaders().find(name);
}
