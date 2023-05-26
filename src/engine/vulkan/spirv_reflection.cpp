#include "spirv_reflection.hpp"
#include "../utils/exceptions.hpp"
#include <spirv_glsl.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const std::unordered_map<spirv_cross::SPIRType::BaseType, VkFormat> spirvTypeToVkFormat = {
    {spirv_cross::SPIRType::Boolean, VkFormat::VK_FORMAT_R32_UINT},
    {spirv_cross::SPIRType::Char, VkFormat::VK_FORMAT_R8_UINT},
    {spirv_cross::SPIRType::Int, VkFormat::VK_FORMAT_R32_SINT},
    {spirv_cross::SPIRType::UInt, VkFormat::VK_FORMAT_R32_UINT},
    {spirv_cross::SPIRType::Half, VkFormat::VK_FORMAT_R16_SFLOAT},
    {spirv_cross::SPIRType::Float, VkFormat::VK_FORMAT_R32_SFLOAT},
    {spirv_cross::SPIRType::Double, VkFormat::VK_FORMAT_R64_SFLOAT},
};

static const std::unordered_map<VkFormat, std::unordered_map<uint32_t, VkFormat>> columnsToFormat = {
    {
        VkFormat::VK_FORMAT_R8_SINT,
        {
            {2, VkFormat::VK_FORMAT_R32G32_SINT},
            {3, VkFormat::VK_FORMAT_R32G32B32_SINT},
            {4, VkFormat::VK_FORMAT_R32G32B32A32_SINT},
        },
    },
    {
        VkFormat::VK_FORMAT_R32_UINT,
        {
            {2, VkFormat::VK_FORMAT_R32G32_UINT},
            {3, VkFormat::VK_FORMAT_R32G32B32_UINT},
            {4, VkFormat::VK_FORMAT_R32G32B32A32_UINT},
        },
    },
    {
        VkFormat::VK_FORMAT_R16_SFLOAT,
        {
            {2, VkFormat::VK_FORMAT_R16G16_SFLOAT},
            {3, VkFormat::VK_FORMAT_R16G16B16_SFLOAT},
            {4, VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT},
        },
    },
    {
        VkFormat::VK_FORMAT_R32_SFLOAT,
        {
            {2, VkFormat::VK_FORMAT_R32G32_SFLOAT},
            {3, VkFormat::VK_FORMAT_R32G32B32_SFLOAT},
            {4, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT},
        },
    },
    {
        VkFormat::VK_FORMAT_R64_SFLOAT,
        {
            {2, VkFormat::VK_FORMAT_R64G64_SFLOAT},
            {3, VkFormat::VK_FORMAT_R64G64B64_SFLOAT},
            {4, VkFormat::VK_FORMAT_R64G64B64A64_SFLOAT},
        },
    },
};

static VkFormat findVkFormat(const spirv_cross::SPIRType& type) {
    const auto format = spirvTypeToVkFormat.find(type.basetype);
    if (format == spirvTypeToVkFormat.end()) {
        return VkFormat::VK_FORMAT_UNDEFINED;
    }

    if (type.vecsize > 1) {
        const auto columns = columnsToFormat.find(format->second);
        if (columns == columnsToFormat.end()) {
            EXCEPTION("Unsupported format: {} with columns: {}", format->second, type.vecsize);
        }

        const auto vectored = columns->second.find(type.vecsize);
        if (vectored == columns->second.end()) {
            EXCEPTION("Unsupported format: {} with columns: {}", format->second, type.vecsize);
        }

        return vectored->second;
    }

    return format->second;
}

SpirvReflection::SpirvReflection(const std::vector<uint32_t>& spriv) :
    compiler{std::make_unique<spirv_cross::CompilerGLSL>(spriv)} {

    auto opts = compiler->get_common_options();
    opts.enable_420pack_extension = true;
    compiler->set_common_options(opts);

    // Reflect on all resource bindings.
    auto resources = compiler->get_shader_resources();

    for (const auto& resource : resources.stage_inputs) {
        inputs.emplace_back();
        auto& input = inputs.back();

        const auto& type = compiler->get_type_from_variable(resource.id);

        input.format = findVkFormat(type);
        input.name = resource.name;
        input.location = compiler->get_decoration(resource.id, spv::DecorationLocation);
    }

    for (const auto& resource : resources.uniform_buffers) {
        uniforms.emplace_back();
        auto& uniform = uniforms.back();

        const auto& type = compiler->get_type_from_variable(resource.id);

        uniform.binding = compiler->get_decoration(resource.id, spv::DecorationBinding);
        uniform.size = compiler->get_declared_struct_size(type);
        uniform.name = resource.name;

        if (!type.array.empty()) {
            EXCEPTION("Array uniforms are not supported");
        }

        if (compiler->get_decoration(resource.id, spv::DecorationDescriptorSet) != 0) {
            EXCEPTION("Uniforms with set number other than 0 are not supported");
        }
    }

    for (const auto& resource : resources.storage_buffers) {
        storageBuffers.emplace_back();
        auto& storageBuffer = storageBuffers.back();

        const auto& type = compiler->get_type_from_variable(resource.id);

        storageBuffer.binding = compiler->get_decoration(resource.id, spv::DecorationBinding);
        storageBuffer.name = resource.name;

        if (!type.array.empty()) {
            EXCEPTION("Array storage buffers are not supported");
        }

        if (compiler->get_decoration(resource.id, spv::DecorationDescriptorSet) != 0) {
            EXCEPTION("Storage buffers with set number other than 0 are not supported");
        }
    }

    for (const auto& resource : resources.sampled_images) {
        samplers.emplace_back();
        auto& sampler = samplers.back();

        const auto& type = compiler->get_type_from_variable(resource.id);
        sampler.name = resource.name;
        sampler.binding = compiler->get_decoration(resource.id, spv::DecorationBinding);

        if (!type.array.empty()) {
            EXCEPTION("Sampler: {} can not be of array type", resource.name);
        }
    }

    for (const auto& resource : resources.subpass_inputs) {
        subpassInputs.emplace_back();
        auto& subpassInput = subpassInputs.back();

        const auto& type = compiler->get_type_from_variable(resource.id);
        subpassInput.name = resource.name;
        subpassInput.binding = compiler->get_decoration(resource.id, spv::DecorationBinding);

        if (!type.array.empty()) {
            EXCEPTION("Sampler: {} can not be of array type", resource.name);
        }
    }

    if (resources.push_constant_buffers.size() > 1) {
        EXCEPTION("Only one push constants buffer is supported");
    }

    if (!resources.push_constant_buffers.empty()) {
        const auto& resource = resources.push_constant_buffers.front();

        const auto& type = compiler->get_type_from_variable(resource.id);

        pushConstants.size = compiler->get_declared_struct_size(type);

        for (size_t i = 0; i < type.member_types.size(); i++) {
            pushConstants.fields.emplace_back();
            auto& field = pushConstants.fields.back();

            const auto& memberType = compiler->get_type(type.member_types[i]);

            field.name = compiler->get_member_name(type.self, i);
            field.format = findVkFormat(memberType);
            field.offset = compiler->type_struct_member_offset(type, i);
            field.size = compiler->get_declared_struct_member_size(type, i);

            if (field.format == VkFormat::VK_FORMAT_UNDEFINED) {
                EXCEPTION("Push constants member: {} has unknown type", field.name);
            }
        }
    }
}

SpirvReflection::~SpirvReflection() = default;

SpirvReflection::SpirvReflection(SpirvReflection&& other) noexcept = default;

SpirvReflection& SpirvReflection::operator=(SpirvReflection&& other) noexcept = default;
