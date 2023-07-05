#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "render_pass_planet_normal.hpp"

using namespace Engine;

RenderSubpassPlanetNormal::RenderSubpassPlanetNormal(VulkanRenderer& vulkan, RenderResources& resources,
                                                     AssetsManager& assetsManager) :
    vulkan{vulkan},
    resources{resources},
    pipelineNormal{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("planet_normal_vert"),
            assetsManager.getShaders().find("planet_normal_frag"),
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
}

void RenderSubpassPlanetNormal::render(VulkanCommandBuffer& vkb, const VulkanTexture& heightmapTexture,
                                       const float resolution, const PlanetTypePtr& planetType) {
    pipelineNormal.getDescriptorPool().reset();

    pipelineNormal.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"textureHeightmap", heightmapTexture};

    pipelineNormal.bindDescriptors(vkb, {}, textures, {});

    const float waterLevel = planetType->getAtmosphere().waterLevel;
    const float strength = glm::clamp(map(resolution, 128.0f, 2048.0f, 0.2f, 0.8f), 0.1f, 1.0f);
    pipelineNormal.pushConstants(vkb,
                                 PushConstant{"resolution", resolution},
                                 PushConstant{"waterLevel", waterLevel},
                                 PushConstant{"strength", strength});

    pipelineNormal.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
