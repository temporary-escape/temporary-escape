// Source: https://github.com/GPUOpen-LibrariesAndSDKs/V-EZ/
//
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <fstream>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/Public/ShaderLang.h>
#include <string>
// #include <glslang/OSDependent/osinclude.h>
#include "../utils/log.hpp"
#include "../utils/string_utils.hpp"
#include "glsl_compiler.hpp"
#include <glslang/SPIRV/GLSL.std.450.h>
#include <glslang/SPIRV/GlslangToSpv.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

const TBuiltInResource defaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .MaxMeshOutputVerticesNV = */ 256,
    /* .MaxMeshOutputPrimitivesNV = */ 512,
    /* .MaxMeshWorkGroupSizeX_NV = */ 32,
    /* .MaxMeshWorkGroupSizeY_NV = */ 1,
    /* .MaxMeshWorkGroupSizeZ_NV = */ 1,
    /* .MaxTaskWorkGroupSizeX_NV = */ 32,
    /* .MaxTaskWorkGroupSizeY_NV = */ 1,
    /* .MaxTaskWorkGroupSizeZ_NV = */ 1,
    /* .MaxMeshViewCountNV = */ 4,
    /* .MaxMeshOutputVerticesEXT = */ 256,
    /* .MaxMeshOutputPrimitivesEXT = */ 512,
    /* .MaxMeshWorkGroupSizeX_EXT = */ 32,
    /* .MaxMeshWorkGroupSizeY_EXT = */ 1,
    /* .MaxMeshWorkGroupSizeZ_EXT = */ 1,
    /* .MaxTaskWorkGroupSizeX_EXT = */ 32,
    /* .MaxTaskWorkGroupSizeY_EXT = */ 1,
    /* .MaxTaskWorkGroupSizeZ_EXT = */ 1,
    /* .MaxMeshViewCountEXT = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,
    /* .limits = */
    {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

enum TOptions {
    EOptionNone = 0,
    EOptionIntermediate = (1 << 0),
    EOptionSuppressInfolog = (1 << 1),
    EOptionMemoryLeakMode = (1 << 2),
    EOptionRelaxedErrors = (1 << 3),
    EOptionGiveWarnings = (1 << 4),
    EOptionLinkProgram = (1 << 5),
    EOptionMultiThreaded = (1 << 6),
    EOptionDumpConfig = (1 << 7),
    EOptionDumpReflection = (1 << 8),
    EOptionSuppressWarnings = (1 << 9),
    EOptionDumpVersions = (1 << 10),
    EOptionSpv = (1 << 11),
    EOptionHumanReadableSpv = (1 << 12),
    EOptionVulkanRules = (1 << 13),
    EOptionDefaultDesktop = (1 << 14),
    EOptionOutputPreprocessed = (1 << 15),
    EOptionOutputHexadecimal = (1 << 16),
    EOptionReadHlsl = (1 << 17),
    EOptionCascadingErrors = (1 << 18),
    EOptionAutoMapBindings = (1 << 19),
    EOptionFlattenUniformArrays = (1 << 20),
    EOptionNoStorageFormat = (1 << 21),
    EOptionKeepUncalled = (1 << 22),
    EOptionHlslOffsets = (1 << 23),
    EOptionHlslIoMapping = (1 << 24),
    EOptionAutoMapLocations = (1 << 25),
    EOptionDebug = (1 << 26),
    EOptionStdin = (1 << 27),
    EOptionOptimizeDisable = (1 << 28),
    EOptionOptimizeSize = (1 << 29),
};

struct ShaderCompUnit {
    EShLanguage stage;
    std::string text;

    // Need to have a special constructors to adjust the fileNameList, since back end needs a list of ptrs
    ShaderCompUnit(EShLanguage istage, const std::string& itext) {
        stage = istage;
        text = itext;
    }

    ShaderCompUnit(const ShaderCompUnit& rhs) {
        stage = rhs.stage;
        text = rhs.text;
    }
};

void SetMessageOptions(int options, EShMessages& messages) {
    if (options & EOptionRelaxedErrors)
        messages = (EShMessages)(messages | EShMsgRelaxedErrors);
    if (options & EOptionIntermediate)
        messages = (EShMessages)(messages | EShMsgAST);
    if (options & EOptionSuppressWarnings)
        messages = (EShMessages)(messages | EShMsgSuppressWarnings);
    if (options & EOptionSpv)
        messages = (EShMessages)(messages | EShMsgSpvRules);
    if (options & EOptionVulkanRules)
        messages = (EShMessages)(messages | EShMsgVulkanRules);
    if (options & EOptionOutputPreprocessed)
        messages = (EShMessages)(messages | EShMsgOnlyPreprocessor);
    if (options & EOptionReadHlsl)
        messages = (EShMessages)(messages | EShMsgReadHlsl);
    if (options & EOptionCascadingErrors)
        messages = (EShMessages)(messages | EShMsgCascadingErrors);
    if (options & EOptionKeepUncalled)
        messages = (EShMessages)(messages | EShMsgKeepUncalled);
}

EShLanguage MapShaderStage(VkShaderStageFlagBits stage) {
    switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
        return EShLangVertex;

    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        return EShLangTessControl;

    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        return EShLangTessEvaluation;

    case VK_SHADER_STAGE_GEOMETRY_BIT:
        return EShLangGeometry;

    case VK_SHADER_STAGE_FRAGMENT_BIT:
        return EShLangFragment;

    case VK_SHADER_STAGE_COMPUTE_BIT:
        return EShLangCompute;

    default:
        return EShLangVertex;
    }
}

bool Engine::compileGLSL2SPIRV(VkShaderStageFlagBits stage, const std::string& source, const std::string& entryPoint,
                               std::vector<uint32_t>& spirv, std::string& infoLog) {
    // Get default built in resource limits.
    const auto& resourceLimits = defaultTBuiltInResource;

    // Initialize glslang library.
    glslang::InitializeProcess();

    // Set message options.
    int options = EOptionSpv | EOptionVulkanRules | EOptionLinkProgram;
    EShMessages messages = EShMsgDefault;
    SetMessageOptions(options, messages);

    // Load GLSL source file into ShaderCompUnit object.
    ShaderCompUnit compUnit(MapShaderStage(stage), source);

    // Create shader from GLSL source.
    const char* fileNameList[1] = {""};
    const char* shaderText = compUnit.text.c_str();
    glslang::TShader shader(compUnit.stage);
    shader.setStringsWithLengthsAndNames(&shaderText, nullptr, fileNameList, 1);
    shader.setEntryPoint(entryPoint.c_str());
    shader.setSourceEntryPoint(entryPoint.c_str());
    shader.setShiftSamplerBinding(0);
    shader.setShiftTextureBinding(0);
    shader.setShiftImageBinding(0);
    shader.setShiftUboBinding(0);
    shader.setShiftSsboBinding(0);
    // shader.setFlattenUniformArrays(false);
    shader.setNoStorageFormat(false);
    if (!shader.parse(&resourceLimits, 100, false, messages)) {
        infoLog = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
        return false;
    }

    // Add shader to new program object.
    glslang::TProgram program;
    program.addShader(&shader);

    // Link program.
    if (!program.link(messages)) {
        infoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        return false;
    }

    // Map IO for SPIRV generation.
    if (!program.mapIO()) {
        infoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        return false;
    }

    // Save any info log that was generated.
    if (shader.getInfoLog())
        infoLog += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";

    if (program.getInfoLog())
        infoLog += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());

    // Translate to SPIRV.
    if (program.getIntermediate(compUnit.stage)) {
        std::string warningsErrors;
        spv::SpvBuildLogger logger;
        glslang::GlslangToSpv(*program.getIntermediate(compUnit.stage), spirv, &logger);
        infoLog += logger.getAllMessages() + "\n";
    }

    // Shutdown glslang library.
    glslang::FinalizeProcess();

    return true;
}

VkShaderStageFlagBits Engine::getGLSLFileFlags(const Path& src) {
    const auto filename = src.filename().stem().string();
    if (endsWith(filename, "_vert")) {
        return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
    } else if (endsWith(filename, "_frag")) {
        return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (endsWith(filename, "_comp")) {
        return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
    } else {
        EXCEPTION("Unknown shader type: '{}'", filename);
    }
}

void Engine::compileGLSLFile(const Path& src, const Path& dst) {
    logger.info("Compiling GLSL file: {} to {}", src, dst);

    const auto flags = getGLSLFileFlags(src);
    const auto glsl = readFileStr(src);

    std::vector<uint32_t> spirv;
    std::string infoLog;

    if (!compileGLSL2SPIRV(flags, glsl, "main", spirv, infoLog)) {
        EXCEPTION("Failed to compile shader: '{}' error: {}", src, infoLog);
    }

    writeFileBinary(dst, spirv.data(), spirv.size() * sizeof(uint32_t));
}
