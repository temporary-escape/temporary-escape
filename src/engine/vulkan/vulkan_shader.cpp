#include "vulkan_shader.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
#include "../utils/md5.hpp"
#include "glsl_compiler.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static std::string loadShaderFile(const Path& path) {
    logger.info("Loading shader: '{}'", path);
    return readFileStr(path);
}

VulkanShader::VulkanShader(VulkanDevice& device, const Path& path, VkShaderStageFlagBits stage) :
    device{device.getDevice()}, stage{stage} {

    if (path.extension().string() != ".spirv") {
        EXCEPTION("Failed to load shader file: '{}' error: only SPIRV format is supported", path);
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    const auto binaryData = readFileBinary(path);
    spirv.resize(binaryData.size() / sizeof(uint32_t));
    std::memcpy(spirv.data(), binaryData.data(), binaryData.size());

    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    if (vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        EXCEPTION("Failed to create shader module!");
    }
}

VulkanShader::VulkanShader(VulkanDevice& device, const std::string& glsl, VkShaderStageFlagBits stage) :
    device{device.getDevice()}, stage{stage} {

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    std::string infoLog;

    logger.debug("Compiling GLSL code of size: {}", glsl.size());
    if (!compileGLSL2SPIRV(stage, glsl, "main", spirv, infoLog)) {
        EXCEPTION("Failed to compile shader error: {}", infoLog);
    }

    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    if (vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        EXCEPTION("Failed to create shader module!");
    }
}

VulkanShader::~VulkanShader() {
    destroy();
}

VulkanShader::VulkanShader(VulkanShader&& other) noexcept {
    swap(other);
}

VulkanShader& VulkanShader::operator=(VulkanShader&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanShader::swap(VulkanShader& other) noexcept {
    std::swap(device, other.device);
    std::swap(stage, other.stage);
    std::swap(shaderModule, other.shaderModule);
    std::swap(spirv, other.spirv);
}

void VulkanShader::destroy() {
    if (shaderModule) {
        vkDestroyShaderModule(device, shaderModule, nullptr);
        shaderModule = VK_NULL_HANDLE;
    }
}
