#include "render_subpass_outline.hpp"
#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "render_pass_outline.hpp"
#include "theme.hpp"

using namespace Engine;

RenderSubpassOutline::RenderSubpassOutline(VulkanRenderer& vulkan, RenderResources& resources,
                                           AssetsManager& assetsManager, const VulkanTexture& entityColorTexture) :
    vulkan{vulkan},
    resources{resources},
    entityColorTexture{entityColorTexture},
    pipelineOutline{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("pass_outline_vert"),
            assetsManager.getShaders().find("pass_outline_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<FullScreenVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::Additive,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassOutline::Attachments::Color,
    });

    addPipeline(pipelineOutline);
}

void RenderSubpassOutline::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineOutline.getDescriptorPool().reset();

    pipelineOutline.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"entityColorTexture", entityColorTexture};

    pipelineOutline.bindDescriptors(vkb, {}, textures, {});

    const auto selected = scene.getSelectedEntity();

    const auto selectedColor = selected ? entityColor(selected->getHandle()) : Vector4{1.0f};
    const auto finalColor = selected ? Theme::primary * alpha(0.6) : Vector4{0.0f};
    const auto thickness = 1.0f;
    pipelineOutline.pushConstants(vkb,
                                  PushConstant{"selectedColor", selectedColor},
                                  PushConstant{"finalColor", finalColor},
                                  PushConstant{"thickness", thickness});

    pipelineOutline.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
