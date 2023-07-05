#include "render_subpass_blur.hpp"
#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassBlur::RenderSubpassBlur(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                     const VulkanTexture& color, const bool vertical) :
    vulkan{vulkan},
    resources{resources},
    color{color},
    vertical{vertical},
    pipelineBlur{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("pass_blur_vert"),
            assetsManager.getShaders().find("pass_blur_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<FullScreenVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
        },
    } {

    setAttachments({
        0,
    });

    addPipeline(pipelineBlur);

    createGaussianKernel(15, 3.5);
}

void RenderSubpassBlur::reset() {
    pipelineBlur.getDescriptorPool().reset();
}

void RenderSubpassBlur::render(VulkanCommandBuffer& vkb) {
    pipelineBlur.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"texSourceColor", color};

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"GaussianWeights", weightsUbo};

    pipelineBlur.bindDescriptors(vkb, uniforms, textures, {});

    const auto horizontal = !vertical;
    pipelineBlur.pushConstants(vkb, PushConstant{"horizontal", horizontal});

    pipelineBlur.renderMesh(vkb, resources.getMeshFullScreenQuad());
}

void RenderSubpassBlur::createGaussianKernel(const size_t size, double sigma) {
    const auto weights = gaussianKernel((size - 1) * 2 + 1, sigma);

    GaussianWeightsUniform data;

    for (size_t i = 0; i < size; i++) {
        const auto w = static_cast<float>(weights[size - i - 1]);
        data.weight[i] = Vector4{w, w, w, 1.0f};
    }
    data.count = static_cast<int>(size);

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(GaussianWeightsUniform);
    bufferInfo.usage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    weightsUbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(weightsUbo, &data, bufferInfo.size);
}
