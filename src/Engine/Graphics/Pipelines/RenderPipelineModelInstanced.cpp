#include "RenderPipelineModelInstanced.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentModel.hpp"
#include <component_model_frag.spirv.h>
#include <component_model_instanced_vert.spirv.h>

using namespace Engine;

RenderPipelineModelInstanced::RenderPipelineModelInstanced(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineModel"} {

    addShader(Embed::component_model_instanced_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_model_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::InstancedVertex>(
        1, VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}
