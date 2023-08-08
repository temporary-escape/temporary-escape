#include "render_pipeline_planet_color.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelinePlanetColor::RenderPipelinePlanetColor(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelinePlanetColor"} {

    addShader(assetsManager.getShaders().find("planet_color_vert"));
    addShader(assetsManager.getShaders().find("planet_color_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelinePlanetColor::setTextureBiome(const VulkanTexture& texture) {
    textures[0] = {"textureBiome", texture};
}

void RenderPipelinePlanetColor::setTextureRoughness(const VulkanTexture& texture) {
    textures[1] = {"textureRoughness", texture};
}

void RenderPipelinePlanetColor::setInputHeightmap(const VulkanTexture& texture) {
    inputs[0] = {"samplerHeightmap", texture};
}

void RenderPipelinePlanetColor::setInputMoisture(const VulkanTexture& texture) {
    inputs[1] = {"samplerMoisture", texture};
}

void RenderPipelinePlanetColor::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, inputs);
}
