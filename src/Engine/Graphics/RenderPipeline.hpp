#pragma once

#include "../Assets/Shader.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Vulkan/SpirvReflection.hpp"
#include "Mesh.hpp"
#include "RenderBuffer.hpp"

namespace Engine {
template <typename T> struct ENGINE_API PushConstant {
    PushConstant(std::string_view name, const T& value) : name{name}, value{value} {
    }

    std::string_view name;
    const T& value;
};

struct ENGINE_API UniformBindingRef {
    UniformBindingRef() = default;
    UniformBindingRef(std::string_view name, const VulkanBuffer& uniform) :
        name{name}, uniform{&uniform}, range{uniform.getSize()} {
    }
    UniformBindingRef(std::string_view name, const VulkanBuffer& uniform, const VkDeviceSize offset,
                      const VkDeviceSize range) :
        name{name}, uniform{&uniform}, offset{offset}, range{range} {
    }

    std::string_view name;
    const VulkanBuffer* uniform{nullptr};
    VkDeviceSize offset{0};
    VkDeviceSize range{0};
};

struct ENGINE_API SamplerBindingRef {
    SamplerBindingRef() = default;
    SamplerBindingRef(std::string_view name, const VulkanTexture& texture) : name{name}, texture{&texture} {
    }

    std::string_view name;
    const VulkanTexture* texture{nullptr};
};

struct ENGINE_API SubpassInputBindingRef {
    SubpassInputBindingRef() = default;
    SubpassInputBindingRef(std::string_view name, const VulkanTexture& texture) : name{name}, texture{&texture} {
    }

    std::string_view name;
    const VulkanTexture* texture{nullptr};
};

class ENGINE_API RenderPipeline {
public:
    enum class DepthMode {
        Ignore,
        Write,
        Read,
        ReadWrite,
    };

    enum class Blending {
        None,
        Normal,
        Additive,
    };

    enum class Stencil {
        None,
        Write,
        Read,
    };

    enum class DepthClamp {
        Disabled,
        Enabled,
    };

    struct VertexInput {
        uint32_t binding;
        VulkanVertexLayoutMap layout;
        size_t size;
        VkVertexInputRate inputRate{VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX};

        template <typename T>
        static VertexInput of(uint32_t binding = 0,
                              VkVertexInputRate inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX) {
            return {binding, T::getLayout(), sizeof(T), inputRate};
        }
    };

    explicit RenderPipeline(VulkanRenderer& vulkan, std::string name);
    virtual ~RenderPipeline() = default;
    NON_MOVEABLE(RenderPipeline);
    NON_COPYABLE(RenderPipeline);

    void create(VulkanRenderPass& renderPass, uint32_t subpass, const std::vector<uint32_t>& attachments);
    bool isCompute() const {
        return compute;
    }
    const std::string& getName() const {
        return name;
    }
    VulkanDescriptorPool& getDescriptionPool();
    void bind(VulkanCommandBuffer& vkb);
    void flushConstants(VulkanCommandBuffer& vkb) {
        pushConstantsBuffer(vkb, constantsBuffer);
    }
    void renderMesh(VulkanCommandBuffer& vkb, const Mesh& mesh) const;
    void renderMeshInstanced(VulkanCommandBuffer& vkb, const Mesh& mesh, const VulkanBuffer& vbo, uint32_t count) const;

protected:
    void addShader(const ShaderPtr& shader);
    void addShader(VulkanShader& shader);
    void addShader(const Span<uint8_t>& spirv, VkShaderStageFlagBits stage);
    void addVertexInput(const VertexInput& vertexInput);
    void setTopology(VkPrimitiveTopology topology);
    void setPolygonMode(VkPolygonMode polygonMode);
    void setCullMode(VkCullModeFlags cullMode);
    void setFrontFace(VkFrontFace frontFace);
    void setDepthMode(DepthMode depthMode);
    void setStencil(Stencil stencil, int stencilValue);
    void setDepthClamp(DepthClamp depthClamp);
    void setCompute(bool value);
    void setBlending(Blending blending);
    void setLineWidth(float width);

    template <typename... Constants> void pushConstants(Constants&&... constants) {
        pushConstantsInternal(constantsBuffer, std::forward<Constants>(constants)...);
    }

    void bindDescriptors(VulkanCommandBuffer& vkb, const Span<UniformBindingRef>& uniforms,
                         const Span<SamplerBindingRef>& textures, const Span<SubpassInputBindingRef>& inputs);

private:
    struct ReflectInfo {
        std::vector<VulkanStageInput> vertexInputs;
        std::unordered_map<std::string, VulkanStageUniform> uniforms;
        std::vector<VulkanStageSampler> samplers;
        std::vector<VulkanStageSampler> subpassInputs;
        std::vector<VulkanStageStorageBuffer> storageBuffers;
        VulkanStagePushConstants pushConstants{};
    };

    template <typename Constant> void pushConstantsInternal(char* dst, const Constant& constant) {
        const auto it = pushConstantsMap.find(constant.name);
        if (it == pushConstantsMap.end()) {
            EXCEPTION("No such constant name: '{}'", constant.name);
        }
        std::memcpy(dst + it->second.offset, &constant.value, it->second.size);
    }

    template <typename Constant, typename... Constants>
    void pushConstantsInternal(char* dst, const Constant& constant, Constants&&... constants) {
        pushConstantsInternal(dst, constant);
        pushConstantsInternal(dst, std::forward<Constants>(constants)...);
    }

    ReflectInfo reflect() const;
    void createGraphicsPipeline(VulkanRenderPass& renderPass, const ReflectInfo& resources, uint32_t subpass,
                                const std::vector<uint32_t>& attachments);
    void createComputePipeline(const ReflectInfo& resources);
    void createDescriptorSetLayout(const ReflectInfo& resources);
    void createDescriptorPool(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings);
    void processPushConstants(const ReflectInfo& resources);
    void pushConstantsBuffer(VulkanCommandBuffer& vkb, const char* src);
    uint32_t findBinding(const std::string_view& name);

    VulkanRenderer& vulkan;
    const std::string name;
    bool compute{false};
    std::list<VulkanShader> compiled;
    std::vector<VulkanShader*> shaders;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    VulkanPipeline::CreateInfo pipelineInfo;
    VulkanPipeline pipeline;
    VulkanDescriptorSetLayout descriptorSetLayout;
    std::array<VulkanDescriptorPool, MAX_FRAMES_IN_FLIGHT> descriptorPools;
    char constantsBuffer[128];
    Blending attachmentBlending;
    std::map<std::string, VulkanStagePushMember, std::less<>> pushConstantsMap;
    std::map<std::string, uint32_t, std::less<>> bindingsMap;
    size_t pushConstantsSize{0};

    struct {
        std::array<VkWriteDescriptorSet, 32> writes;
        std::array<VkDescriptorBufferInfo, 16> buffers;
        std::array<VkDescriptorImageInfo, 16> images;
    } descriptors;
};
} // namespace Engine
