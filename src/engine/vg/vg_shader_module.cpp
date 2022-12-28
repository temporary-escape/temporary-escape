#include "vg_shader_module.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
#include "../utils/md5.hpp"
#include "glsl_compiler.hpp"

#define CMP "VgShaderModule"

using namespace Engine;

static std::string loadShaderFile(const Path& path) {
    Log::i(CMP, "Loading shader: '{}'", path);
    return readFileStr(path);
}

VgShaderModule::VgShaderModule(const Config& config, VkDevice device, const Path& path, VkShaderStageFlagBits stage) :
    VgShaderModule{config, device, loadShaderFile(path), stage} {
}

VgShaderModule::VgShaderModule(const Config& config, VkDevice device, const std::string& glsl,
                               VkShaderStageFlagBits stage) :
    device{device}, stage{stage} {

    const auto codeMd5 = md5sum(glsl.data(), glsl.size());
    const auto binaryPath = config.shaderCachePath / (codeMd5 + ".bin");

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    std::vector<char> binaryData;
    std::vector<uint32_t> spirv;

    // Is shader cached?
    if (Fs::exists(binaryPath) && Fs::is_regular_file(binaryPath)) {
        Log::i(CMP, "Loading shader module binary: '{}'", binaryPath);
        binaryData = readFileBinary(binaryPath);

        createInfo.codeSize = binaryData.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(binaryData.data());
    }

    // Shader was not cached, we need to compile it!
    else {
        std::string infoLog;

        Log::d(CMP, "Compiling GLSL code of size: {}", glsl.size());
        if (!CompileGLSL2SPIRV(stage, glsl, "main", spirv, infoLog)) {
            EXCEPTION("Failed to compile shader error: {}", infoLog);
        }

        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pCode = spirv.data();

        Log::i(CMP, "Writing shader module binary: '{}'", binaryPath);
        writeFileBinary(binaryPath, spirv.data(), spirv.size() * sizeof(uint32_t));
    }

    Log::d(CMP, "Creating shader module stage: {} size: {}", stage, createInfo.codeSize);

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        EXCEPTION("Failed to create shader module!");
    }
}

VgShaderModule::~VgShaderModule() {
    cleanup();
}

VgShaderModule::VgShaderModule(VgShaderModule&& other) noexcept {
    swap(other);
}

VgShaderModule& VgShaderModule::operator=(VgShaderModule&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgShaderModule::swap(VgShaderModule& other) noexcept {
    std::swap(device, other.device);
    std::swap(shaderModule, other.shaderModule);
    std::swap(stage, other.stage);
}

void VgShaderModule::cleanup() {
    if (shaderModule) {
        vkDestroyShaderModule(device, shaderModule, nullptr);
        shaderModule = VK_NULL_HANDLE;
    }
}
