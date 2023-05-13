#include "render_subpass_brdf.hpp"
#include "../assets/registry.hpp"
#include "mesh_utils.hpp"
#include "render_pass_brdf.hpp"

using namespace Engine;

RenderSubpassBrdf::RenderSubpassBrdf(VulkanRenderer& vulkan, Registry& registry) :
    vulkan{vulkan},
    pipelineBrdf{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("brdf_vert"),
            registry.getShaders().find("brdf_frag"),
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

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassBrdf::render(VulkanCommandBuffer& vkb) {
    pipelineBrdf.getDescriptorPool().reset();

    pipelineBrdf.bind(vkb);
    pipelineBrdf.renderMesh(vkb, fullScreenQuad);
}
