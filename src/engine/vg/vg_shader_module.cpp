#include "vg_shader_module.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
#include "../utils/md5.hpp"
#include "glsl_compiler.hpp"
#include "vg_renderer.hpp"

#define CMP "VgShaderModule"

using namespace Engine;

static std::string loadShaderFile(const Path& path) {
    Log::i(CMP, "Loading shader: '{}'", path);
    return readFileStr(path);
}

VgShaderModule::VgShaderModule(const Config& config, VgRenderer& renderer, const Path& path,
                               VkShaderStageFlagBits stage) :
    VgShaderModule{config, renderer, loadShaderFile(path), stage} {
}

VgShaderModule::VgShaderModule(const Config& config, VgRenderer& renderer, const std::string& glsl,
                               VkShaderStageFlagBits stage) :
    state{std::make_shared<ShaderModuleState>()} {

    state->renderer = &renderer;
    state->stage = stage;

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

    if (vkCreateShaderModule(renderer.getDevice(), &createInfo, nullptr, &state->shaderModule) != VK_SUCCESS) {
        EXCEPTION("Failed to create shader module!");
    }
}

VgShaderModule::~VgShaderModule() {
    destroy();
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
    std::swap(state, other.state);
}

void VgShaderModule::destroy() {
    if (state && state->renderer) {
        state->renderer->dispose(state);
    }

    state.reset();
}

void VgShaderModule::ShaderModuleState::destroy() {
    if (shaderModule) {
        vkDestroyShaderModule(renderer->getDevice(), shaderModule, nullptr);
        shaderModule = VK_NULL_HANDLE;
    }
}
