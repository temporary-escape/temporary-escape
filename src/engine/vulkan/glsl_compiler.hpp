#pragma once

#include "../library.hpp"
#include "../utils/path.hpp"
#include <string>
#include <vector>
#include <volk.h>

namespace Engine {
extern ENGINE_API bool compileGLSL2SPIRV(VkShaderStageFlagBits stage, const std::string& source,
                                         const std::string& entryPoint, std::vector<uint32_t>& spirv,
                                         std::string& infoLog);

extern ENGINE_API VkShaderStageFlagBits getGLSLFileFlags(const Path& src);
extern ENGINE_API void compileGLSLFile(const Path& src, const Path& dst);
} // namespace Engine
