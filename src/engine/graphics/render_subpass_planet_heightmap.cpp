#include "render_subpass_planet_heightmap.hpp"
#include "../assets/registry.hpp"
#include "../math/random.hpp"
#include "mesh_utils.hpp"
#include "render_pass_planet_surface.hpp"

using namespace Engine;

RenderSubpassPlanetHeightmap::RenderSubpassPlanetHeightmap(VulkanRenderer& vulkan, Registry& registry) :
    vulkan{vulkan},
    pipelineHeightmap{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("planet_flow_noise_vert"),
            registry.getShaders().find("planet_flow_noise_frag"),
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
        RenderPassPlanetSurface::Attachments::Heightmap,
    });

    addPipeline(pipelineHeightmap);

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassPlanetHeightmap::render(VulkanCommandBuffer& vkb, Rng& rng, const int index, const float resolution) {
    pipelineHeightmap.getDescriptorPool().reset();

    pipelineHeightmap.bind(vkb);

    const float resMin = 0.01f;
    const float resMax = 5.0f;

    // {"seed":821.9716048181647,"resolution":1024,"res1":0.3690844724045967,"res2":0.42204962377908944,"resMix":0.468178635794637,"mixScale":0.7266077201316659,"doesRidged":3}
    /*const float seed = 821.9716048181647f;
    const float res1 = 0.3690844724045967f;
    const float res2 = 0.42204962377908944;
    const float resMix = 0.468178635794637f;
    const float mixScale = 0.7266077201316659f;
    const float doesRidged = 3.0f;*/

    const float seed = randomReal(rng, 0.0f, 1.0f) * 1000.0f;
    const float res1 = randomReal(rng, resMin, resMax);
    const float res2 = randomReal(rng, resMin, resMax);
    const float resMix = randomReal(rng, resMin, resMax);
    const float mixScale = randomReal(rng, 0.5f, 1.0f);
    const float doesRidged = std::floor(randomReal(rng, 0.0f, 4.0f));

    pipelineHeightmap.pushConstants(vkb,
                                    PushConstant{"index", index},
                                    PushConstant{"seed", seed},
                                    PushConstant{"resolution", resolution},
                                    PushConstant{"res1", res1},
                                    PushConstant{"res2", res2},
                                    PushConstant{"resMix", resMix},
                                    PushConstant{"mixScale", mixScale},
                                    PushConstant{"doesRidged", doesRidged});

    pipelineHeightmap.renderMesh(vkb, fullScreenQuad);
}
