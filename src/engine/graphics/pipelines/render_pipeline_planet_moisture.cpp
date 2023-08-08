#include "render_pipeline_planet_moisture.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelinePlanetMoisture::RenderPipelinePlanetMoisture(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelinePlanetMoisture"} {

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

void RenderPipelinePlanetMoisture::setIndex(const int value) {
    pushConstants(PushConstant{"index", value});
}

void RenderPipelinePlanetMoisture::setSeed(const float value) {
    pushConstants(PushConstant{"seed", value});
}

void RenderPipelinePlanetMoisture::setResolution(const float value) {
    pushConstants(PushConstant{"resolution", value});
}

void RenderPipelinePlanetMoisture::setRes1(const float value) {
    pushConstants(PushConstant{"res1", value});
}

void RenderPipelinePlanetMoisture::setRes2(const float value) {
    pushConstants(PushConstant{"res2", value});
}

void RenderPipelinePlanetMoisture::setResMix(const float value) {
    pushConstants(PushConstant{"resMix", value});
}

void RenderPipelinePlanetMoisture::setMixScale(const float value) {
    pushConstants(PushConstant{"mixScale", value});
}

void RenderPipelinePlanetMoisture::setDoesRidged(const float value) {
    pushConstants(PushConstant{"doesRidged", value});
}
