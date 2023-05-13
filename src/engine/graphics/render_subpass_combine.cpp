#include "render_subpass_combine.hpp"
#include "../assets/registry.hpp"
#include "mesh_utils.hpp"
#include "render_pass_combine.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassCombine::RenderSubpassCombine(const Config& config, VulkanRenderer& vulkan, Registry& registry,
                                           const VulkanTexture& color, const VulkanTexture& blured) :
    config{config},
    vulkan{vulkan},
    color{color},
    blured{blured},
    pipelineCombine{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("pass_combine_vert"),
            registry.getShaders().find("pass_combine_frag"),
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
        RenderPassCombine::Attachments::Color,
    });

    addPipeline(pipelineCombine);

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassCombine::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineCombine.getDescriptorPool().reset();

    pipelineCombine.bind(vkb);

    std::array<SamplerBindingRef, 2> textures{};
    textures[0] = {"texForwardColor", color};
    textures[1] = {"texBloomColor", blured};

    pipelineCombine.bindDescriptors(vkb, {}, textures, {});

    const auto bloomStrength = config.graphics.bloomStrength;
    const auto bloomPower = config.graphics.bloomPower;
    const auto exposure = config.graphics.exposure;
    const auto gamma = config.graphics.gamma;
    const auto contrast = config.graphics.contrast;

    pipelineCombine.pushConstants(vkb,
                                  PushConstant{"bloomStrength", bloomStrength},
                                  PushConstant{"bloomPower", bloomPower},
                                  PushConstant{"exposure", exposure},
                                  PushConstant{"gamma", gamma},
                                  PushConstant{"contrast", contrast});

    pipelineCombine.renderMesh(vkb, fullScreenQuad);
}
