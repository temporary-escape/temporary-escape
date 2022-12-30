#pragma once

#include "../library.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

extern ENGINE_API bool CompileGLSL2SPIRV(VkShaderStageFlagBits stage, const std::string& source,
                                         const std::string& entryPoint, std::vector<uint32_t>& spirv,
                                         std::string& infoLog);
