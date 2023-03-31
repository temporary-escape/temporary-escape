#include "render_subpass_skybox.hpp"
#include "../assets/registry.hpp"
#include "mesh_utils.hpp"
#include "render_pass_lighting.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassSkybox::RenderSubpassSkybox(VulkanRenderer& vulkan, Registry& registry) :
    vulkan{vulkan},
    pipelineSkybox{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("pass-skybox.vert"),
            registry.getShaders().find("pass-skybox.frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<SkyboxVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassLighting::Attachments::Forward,
    });

    addPipeline(pipelineSkybox);

    cube = createSkyboxCube(vulkan);
}

void RenderSubpassSkybox::render(VulkanCommandBuffer& vkb, Scene& scene) {
    auto camera = scene.getPrimaryCamera();
    auto skybox = scene.getSkybox();
    if (!skybox) {
        EXCEPTION("Scene has no skybox attached");
    }

    pipelineSkybox.getDescriptorPool().reset();

    pipelineSkybox.bind(vkb);

    const auto modelMatrix = glm::scale(Matrix4{1.0f}, Vector3{1000.0f});

    pipelineSkybox.pushConstants(vkb,
                                 // Constants
                                 PushConstant{"modelMatrix", modelMatrix});

    std::array<UniformBindingRef, 1> uniforms;
    std::array<SamplerBindingRef, 1> textures;

    uniforms[0] = {"Camera", camera->getUboZeroPos().getCurrentBuffer()};
    textures[0] = {"texBaseColor", skybox->getTexture()};

    pipelineSkybox.bindDescriptors(vkb, uniforms, textures, {});

    pipelineSkybox.renderMesh(vkb, cube);
}
