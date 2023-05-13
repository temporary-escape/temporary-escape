#include "shader.hpp"
#include "../vulkan/glsl_compiler.hpp"
#include "registry.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

Shader::Shader(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Shader::load(Registry& registry, VulkanRenderer& vulkan) {
    try {
        stage = getGLSLFileFlags(path.filename().string());
        shader = vulkan.createShaderModule(path, stage);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load shader: '{}'", getName());
    }
}

ShaderPtr Shader::from(const std::string& name) {
    return Registry::getInstance().getShaders().find(name);
}
