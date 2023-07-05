#include "render_subpass_brdf.hpp"
#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "render_pass_brdf.hpp"

using namespace Engine;

RenderSubpassBrdf::RenderSubpassBrdf(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager) :
    vulkan{vulkan},
    resources{resources},
    pipelineBrdf{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("brdf_vert"),
            assetsManager.getShaders().find("brdf_frag"),
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
        RenderPassBrdf::Attachments::Color,
    });

    addPipeline(pipelineBrdf);
}

void RenderSubpassBrdf::render(VulkanCommandBuffer& vkb) {
    pipelineBrdf.getDescriptorPool().reset();

    pipelineBrdf.bind(vkb);
    pipelineBrdf.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
