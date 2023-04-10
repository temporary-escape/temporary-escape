#pragma once

#include "../assets/shader.hpp"
#include "../utils/exceptions.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "mesh.hpp"

namespace Engine {
template <typename T> struct ENGINE_API PushConstant {
    PushConstant(std::string_view name, const T& value) : name{name}, value{value} {
    }

    std::string_view name;
    const T& value;
};

struct ENGINE_API UniformBindingRef {
    UniformBindingRef() = default;
    UniformBindingRef(std::string_view name, const VulkanBuffer& uniform) : name{name}, uniform{&uniform} {
    }

    std::string_view name;
    const VulkanBuffer* uniform;
};

struct ENGINE_API SamplerBindingRef {
    SamplerBindingRef() = default;
    SamplerBindingRef(std::string_view name, const VulkanTexture& texture) : name{name}, texture{&texture} {
    }

    std::string_view name;
    const VulkanTexture* texture;
};

struct ENGINE_API SubpassInputBindingRef {
    SubpassInputBindingRef() = default;
    SubpassInputBindingRef(std::string_view name, const VulkanTexture& texture) : name{name}, texture{&texture} {
    }

    std::string_view name;
    const VulkanTexture* texture;
};

class ENGINE_API RenderPipeline : public NonCopyable {
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

    struct VertexInput {
        uint32_t binding;
        VulkanVertexLayoutMap layout;
        size_t size;
        VkVertexInputRate inputRate{VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX};

        template <typename T> static VertexInput of(uint32_t binding = 0) {
            return {binding, T::getLayout(), sizeof(T)};
        }
    };

    struct Options {
        VkPrimitiveTopology topology{VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
        DepthMode depth{0};
        Blending blending{0};
        VkPolygonMode polygonMode{VkPolygonMode::VK_POLYGON_MODE_FILL};
        VkCullModeFlags cullMode{VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT};
        VkFrontFace frontFace{VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE};
        Stencil stencil{Stencil::None};
        int stencilValue{0xff};
    };

    explicit RenderPipeline(VulkanRenderer& vulkan, std::vector<ShaderPtr> shaders, std::vector<VertexInput> inputs,
                            Options options);
    explicit RenderPipeline(VulkanRenderer& vulkan, std::vector<ShaderPtr> shaders);
    virtual ~RenderPipeline() = default;
    NON_MOVEABLE(RenderPipeline);

    void init(VulkanRenderPass& renderPass, const std::vector<uint32_t>& attachments, uint32_t subpass = 0);
    void init();

    const VulkanPipeline& getPipeline() const {
        return pipeline;
    }

    const VulkanDescriptorSetLayout& getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }

    const VulkanDescriptorPool& getDescriptorPool() const {
        return descriptorPools[vulkan.getCurrentFrameNum()];
    }

    VulkanDescriptorPool& getDescriptorPool() {
        return descriptorPools[vulkan.getCurrentFrameNum()];
    }

    template <typename... Constants> void pushConstants(VulkanCommandBuffer& vkb, Constants&&... constants) {
        char buffer[128];
        pushConstantsInternal(buffer, std::forward<Constants>(constants)...);
        pushConstantsBuffer(vkb, buffer);
    }

    void bind(VulkanCommandBuffer& vkb);

    void bindDescriptors(VulkanCommandBuffer& vkb, const Span<UniformBindingRef>& uniforms,
                         const Span<SamplerBindingRef>& textures, const Span<SubpassInputBindingRef>& inputs);

    void renderMesh(VulkanCommandBuffer& vkb, const Mesh& mesh);

private:
    struct ReflectInfo {
        std::vector<VulkanStageInput> vertexInputs;
        std::unordered_map<std::string, VulkanStageUniform> uniforms;
        std::vector<VulkanStageSampler> samplers;
        std::vector<VulkanStageSampler> subpassInputs;
        std::vector<VulkanStageStorageBuffer> storageBuffers;
        VulkanStagePushConstants pushConstants{};
    };

    ReflectInfo reflect();
    void processPushConstants(const ReflectInfo& resources);
    void createDescriptorSetLayout(const ReflectInfo& resources);
    void createPipeline(const ReflectInfo& resources, VulkanRenderPass& renderPass,
                        const std::vector<uint32_t>& attachments, uint32_t subpass);
    void createPipeline(const ReflectInfo& resources);
    void createDescriptorPool(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings);

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
    void pushConstantsBuffer(VulkanCommandBuffer& vkb, const char* src);
    uint32_t findBinding(const std::string_view& name);

    VulkanRenderer& vulkan;
    std::string id;

    struct CreateInfo {
        std::vector<ShaderPtr> shaders;
        std::vector<VertexInput> inputs;
        Options options;
    } createInfo;

    VulkanPipeline pipeline;
    VulkanDescriptorSetLayout descriptorSetLayout;

    std::map<std::string, VulkanStagePushMember, std::less<>> pushConstantsMap;
    std::map<std::string, uint32_t, std::less<>> bindingsMap;
    size_t pushConstantsSize{0};

    std::array<VulkanDescriptorPool, MAX_FRAMES_IN_FLIGHT> descriptorPools;

    struct {
        std::array<VkWriteDescriptorSet, 32> writes;
        std::array<VkDescriptorBufferInfo, 16> buffers;
        std::array<VkDescriptorImageInfo, 16> images;
    } descriptors;
};
} // namespace Engine
