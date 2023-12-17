#include "RenderPipelinePlanetHeight.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <planet_flow_noise_frag.spirv.h>
#include <planet_flow_noise_vert.spirv.h>

using namespace Engine;

RenderPipelinePlanetHeight::RenderPipelinePlanetHeight(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelinePlanetHeight"} {

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
