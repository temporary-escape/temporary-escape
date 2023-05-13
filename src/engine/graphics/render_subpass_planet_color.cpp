#include "render_subpass_planet_color.hpp"
#include "../assets/registry.hpp"
#include "../math/random.hpp"
#include "mesh_utils.hpp"
#include "render_pass_planet_surface.hpp"

using namespace Engine;

RenderSubpassPlanetColor::RenderSubpassPlanetColor(VulkanRenderer& vulkan, Registry& registry,
                                                   RenderPassPlanetSurface& parent) :
    vulkan{vulkan},
    parent{parent},
    pipelineColor{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("planet_color_vert"),
            registry.getShaders().find("planet_color_frag"),
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
        RenderPassPlanetSurface::Attachments::Color,
        RenderPassPlanetSurface::Attachments::MetallicRoughness,
    });

    setInputs({
        RenderPassPlanetSurface::Attachments::Heightmap,
        RenderPassPlanetSurface::Attachments::Moisture,
    });

    addPipeline(pipelineColor);

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassPlanetColor::render(VulkanCommandBuffer& vkb, const PlanetTypePtr& planetType) {
    pipelineColor.getDescriptorPool().reset();

    pipelineColor.bind(vkb);

    std::array<SamplerBindingRef, 2> textures{};
    textures[0] = {"textureBiome", planetType->getBiomeTexture()->getVulkanTexture()};
    textures[1] = {"textureRoughness", planetType->getRoughnessTexture()->getVulkanTexture()};

    std::array<SubpassInputBindingRef, 2> inputs{};
    inputs[0] = {"samplerHeightmap", parent.getTexture(RenderPassPlanetSurface::Attachments::Heightmap)};
    inputs[1] = {"samplerMoisture", parent.getTexture(RenderPassPlanetSurface::Attachments::Moisture)};

    pipelineColor.bindDescriptors(vkb, {}, textures, inputs);

    pipelineColor.renderMesh(vkb, fullScreenQuad);
}
