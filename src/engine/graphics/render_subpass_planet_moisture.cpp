#include "render_subpass_planet_moisture.hpp"
#include "../assets/assets_manager.hpp"
#include "../math/random.hpp"
#include "mesh_utils.hpp"
#include "render_pass_planet_surface.hpp"

using namespace Engine;

RenderSubpassPlanetMoisture::RenderSubpassPlanetMoisture(VulkanRenderer& vulkan, RenderResources& resources,
                                                         AssetsManager& assetsManager) :
    vulkan{vulkan},
    resources{resources},
    pipelineMoisture{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("planet_flow_noise_vert"),
            assetsManager.getShaders().find("planet_flow_noise_frag"),
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
        RenderPassPlanetSurface::Attachments::Moisture,
    });

    addPipeline(pipelineMoisture);
}

void RenderSubpassPlanetMoisture::render(VulkanCommandBuffer& vkb, Rng& rng, const int index, const float resolution) {
    pipelineMoisture.getDescriptorPool().reset();

    pipelineMoisture.bind(vkb);

    const float resMin = 0.01f;
    const float resMax = 5.0f;

    const float seed = randomReal(rng, 0.0f, 1.0f) * 1000.0f;
    const float res1 = randomReal(rng, resMin, resMax);
    const float res2 = randomReal(rng, resMin, resMax);
    const float resMix = randomReal(rng, resMin, resMax);
    const float mixScale = randomReal(rng, 0.5f, 1.0f);
    const float doesRidged = std::floor(randomReal(rng, 0.0f, 4.0f));

    pipelineMoisture.pushConstants(vkb,
                                   PushConstant{"index", index},
                                   PushConstant{"seed", seed},
                                   PushConstant{"resolution", resolution},
                                   PushConstant{"res1", res1},
                                   PushConstant{"res2", res2},
                                   PushConstant{"resMix", resMix},
                                   PushConstant{"mixScale", mixScale},
                                   PushConstant{"doesRidged", doesRidged});

    pipelineMoisture.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
