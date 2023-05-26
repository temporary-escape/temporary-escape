#include "render_subpass_bloom_extract.hpp"
#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "render_pass_bloom.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassBloomExtract::RenderSubpassBloomExtract(VulkanRenderer& vulkan, AssetsManager& assetsManager,
                                                     const VulkanTexture& source) :
    vulkan{vulkan},
    source{source},
    pipelineBloomExtract{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("pass_bloom_extract_vert"),
            assetsManager.getShaders().find("pass_bloom_extract_frag"),
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
        RenderPassBloom::Extract::Attachments::Color,
    });

    addPipeline(pipelineBloomExtract);

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassBloomExtract::render(VulkanCommandBuffer& vkb) {
    pipelineBloomExtract.getDescriptorPool().reset();

    pipelineBloomExtract.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"texSourceColor", source};

    pipelineBloomExtract.bindDescriptors(vkb, {}, textures, {});

    const auto threshold = 0.3f;
    pipelineBloomExtract.pushConstants(vkb, PushConstant{"threshold", threshold});

    pipelineBloomExtract.renderMesh(vkb, fullScreenQuad);
}
