#include "vulkan_shader_module.hpp"
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

VulkanShaderModule::VulkanShaderModule(const Config& config, VulkanDevice& device, const Path& path,
                                       VkShaderStageFlagBits stage) :
    VulkanShaderModule{config, device, loadShaderFile(path), stage} {
}

VulkanShaderModule::VulkanShaderModule(const Config& config, VulkanDevice& device, const std::string& glsl,
                                       VkShaderStageFlagBits stage) :
    device{device.getDevice()}, stage{stage} {

    const auto codeMd5 = md5sum(glsl.data(), glsl.size());
    const auto binaryPath = config.shaderCachePath / (codeMd5 + ".bin");

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    std::vector<char> binaryData;
    std::vector<uint32_t> spirv;

    // Is shader cached?
    if (Fs::exists(binaryPath) && Fs::is_regular_file(binaryPath)) {
        logger.info("Loading shader module binary: '{}'", binaryPath);
        binaryData = readFileBinary(binaryPath);

        createInfo.codeSize = binaryData.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(binaryData.data());
    }

    // Shader was not cached, we need to compile it!
    else {
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

VulkanShaderModule::~VulkanShaderModule() {
    destroy();
}

VulkanShaderModule::VulkanShaderModule(VulkanShaderModule&& other) noexcept {
    swap(other);
}

VulkanShaderModule& VulkanShaderModule::operator=(VulkanShaderModule&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanShaderModule::swap(VulkanShaderModule& other) noexcept {
    std::swap(device, other.device);
    std::swap(stage, other.stage);
    std::swap(shaderModule, other.shaderModule);
}

void VulkanShaderModule::destroy() {
    if (shaderModule) {
        vkDestroyShaderModule(device, shaderModule, nullptr);
        shaderModule = VK_NULL_HANDLE;
    }
}
