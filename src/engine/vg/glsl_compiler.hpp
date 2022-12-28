#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

extern bool CompileGLSL2SPIRV(VkShaderStageFlagBits stage, const std::string& source, const std::string& entryPoint,
                              std::vector<uint32_t>& spirv, std::string& infoLog);
