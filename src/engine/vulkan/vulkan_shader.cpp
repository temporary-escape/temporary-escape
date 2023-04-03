#include "vulkan_shader.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
#include "../utils/md5.hpp"
#include "glsl_compiler.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

static std::string loadShaderFile(const Path& path) {
    logger.info("Loading shader: '{}'", path);
    return readFileStr(path);
}

VulkanShader::VulkanShader(const Config& config, VulkanDevice& device, const Path& path, VkShaderStageFlagBits stage) :
    VulkanShader{config, device, loadShaderFile(path), stage} {
}

VulkanShader::VulkanShader(const Config& config, VulkanDevice& device, const std::string& glsl,
                           VkShaderStageFlagBits stage) :
    device{device.getDevice()}, stage{stage} {

    const auto codeMd5 = md5sum(glsl.data(), glsl.size());
    const auto binaryPath = config.shaderCachePath / (codeMd5 + ".bin");

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // Is shader cached?
    if (Fs::exists(binaryPath) && Fs::is_regular_file(binaryPath)) {
        logger.info("Loading shader module binary: '{}'", binaryPath);

        const auto binaryData = readFileBinary(binaryPath);
        spirv.resize(binaryData.size() / sizeof(uint32_t));
        std::memcpy(spirv.data(), binaryData.data(), binaryData.size());

        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pCode = spirv.data();
    }

    // Shader was not cached, we need to compile it!
    if (!createInfo.pCode || createInfo.codeSize == 0) {
        std::string infoLog;

        logger.debug("Compiling GLSL code of size: {}", glsl.size());
        if (!CompileGLSL2SPIRV(stage, glsl, "main", spirv, infoLog)) {
            EXCEPTION("Failed to compile shader error: {}", infoLog);
        }

        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pCode = spirv.data();

        logger.info("Writing shader module binary: '{}'", binaryPath);
        writeFileBinary(binaryPath, spirv.data(), spirv.size() * sizeof(uint32_t));
    }

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
