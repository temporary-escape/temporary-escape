#include "render_pipeline_planet_height.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelinePlanetHeight::RenderPipelinePlanetHeight(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelinePlanetHeight"} {

    addShader(assetsManager.getShaders().find("planet_flow_noise_vert"));
    addShader(assetsManager.getShaders().find("planet_flow_noise_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelinePlanetHeight::setIndex(const int value) {
    pushConstants(PushConstant{"index", value});
}

void RenderPipelinePlanetHeight::setSeed(const float value) {
    pushConstants(PushConstant{"seed", value});
}

void RenderPipelinePlanetHeight::setResolution(const float value) {
    pushConstants(PushConstant{"resolution", value});
}

void RenderPipelinePlanetHeight::setRes1(const float value) {
    pushConstants(PushConstant{"res1", value});
}

void RenderPipelinePlanetHeight::setRes2(const float value) {
    pushConstants(PushConstant{"res2", value});
}

void RenderPipelinePlanetHeight::setResMix(const float value) {
    pushConstants(PushConstant{"resMix", value});
}

void RenderPipelinePlanetHeight::setMixScale(const float value) {
    pushConstants(PushConstant{"mixScale", value});
}

void RenderPipelinePlanetHeight::setDoesRidged(const float value) {
    pushConstants(PushConstant{"doesRidged", value});
}
