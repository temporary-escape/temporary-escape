#include "../assets/registry.hpp"
#include "mesh_utils.hpp"
#include "render_pass_planet_normal.hpp"

using namespace Engine;

RenderSubpassPlanetNormal::RenderSubpassPlanetNormal(VulkanRenderer& vulkan, Registry& registry) :
    vulkan{vulkan},
    pipelineNormal{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("planet_normal_vert"),
            registry.getShaders().find("planet_normal_frag"),
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
        RenderPassPlanetNormal::Attachments::Normal,
    });

    addPipeline(pipelineNormal);

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassPlanetNormal::render(VulkanCommandBuffer& vkb, const VulkanTexture& heightmapTexture,
                                       const float resolution, const PlanetTypePtr& planetType) {
    pipelineNormal.getDescriptorPool().reset();

    pipelineNormal.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"textureHeightmap", heightmapTexture};

    pipelineNormal.bindDescriptors(vkb, {}, textures, {});

    const float waterLevel = planetType->getAtmosphere().waterLevel;
    pipelineNormal.pushConstants(vkb, PushConstant{"resolution", resolution}, PushConstant{"waterLevel", waterLevel});

    pipelineNormal.renderMesh(vkb, fullScreenQuad);
}
