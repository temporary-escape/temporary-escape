#include "render_subpass_fxaa.hpp"
#include "../assets/registry.hpp"
#include "mesh_utils.hpp"
#include "render_pass_fxaa.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassFxaa::RenderSubpassFxaa(VulkanRenderer& vulkan, Registry& registry, const VulkanTexture& forward) :
    vulkan{vulkan},
    forward{forward},
    pipelineFxaa{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("pass-fxaa.vert"),
            registry.getShaders().find("pass-fxaa.frag"),
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
        RenderPassFxaa::Attachments::Color,
    });

    addPipeline(pipelineFxaa);

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassFxaa::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineFxaa.getDescriptorPool().reset();

    auto camera = scene.getPrimaryCamera();

    pipelineFxaa.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"texColor", forward};

    pipelineFxaa.bindDescriptors(vkb, {}, textures, {});

    const auto textureSize = Vector2{camera->getViewport()};
    pipelineFxaa.pushConstants(vkb, PushConstant{"textureSize", textureSize});

    pipelineFxaa.renderMesh(vkb, fullScreenQuad);
}
