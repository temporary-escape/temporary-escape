#include "RenderPipelinePlanetMoisture.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <planet_flow_noise_frag.spirv.h>
#include <planet_flow_noise_vert.spirv.h>

using namespace Engine;

RenderPipelinePlanetMoisture::RenderPipelinePlanetMoisture(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelinePlanetMoisture"} {

    addShader(Embed::planet_flow_noise_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::planet_flow_noise_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
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
