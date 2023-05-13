#include "render_subpass_pbr.hpp"
#include "../assets/registry.hpp"
#include "mesh_utils.hpp"
#include "render_pass_lighting.hpp"
#include "render_pass_opaque.hpp"
#include "render_pass_ssao.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassPbr::RenderSubpassPbr(VulkanRenderer& vulkan, Registry& registry, const RenderPassOpaque& opaque,
                                   const RenderPassSsao& ssao, const VulkanTexture& brdf) :
    vulkan{vulkan},
    opaque{opaque},
    ssao{ssao},
    brdf{brdf},
    defaultSkybox{vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}},
    pipelinePbr{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("pass_pbr_vert"),
            registry.getShaders().find("pass_pbr_frag"),
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
        RenderPassLighting::Attachments::Forward,
    });

    addPipeline(pipelinePbr);

    fullScreenQuad = createFullScreenQuad(vulkan);
}

void RenderSubpassPbr::render(VulkanCommandBuffer& vkb, Scene& scene) {
    updateDirectionalLights(scene);
    auto camera = scene.getPrimaryCamera();
    auto skybox = scene.getSkybox();
    if (!skybox) {
        skybox = &defaultSkybox;
    }

    pipelinePbr.getDescriptorPool().reset();

    pipelinePbr.bind(vkb);

    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 8> textures;

    const auto& texSsao = ssao.getTexture(RenderPassSsao::Attachments::Ssao);
    const auto& texDepth = opaque.getTexture(RenderPassOpaque::Attachments::Depth);
    const auto& texBaseColorAmbient = opaque.getTexture(RenderPassOpaque::Attachments::AlbedoAmbient);
    const auto& texEmissiveRoughness = opaque.getTexture(RenderPassOpaque::Attachments::EmissiveRoughness);
    const auto& texNormalMetallic = opaque.getTexture(RenderPassOpaque::Attachments::NormalMetallic);

    uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};
    uniforms[1] = {"DirectionalLights", directionalLightsUbo.getCurrentBuffer()};

    textures[0] = {"texIrradiance", skybox->getIrradiance()};
    textures[1] = {"texPrefilter", skybox->getPrefilter()};
    textures[2] = {"texBrdf", brdf};
    textures[3] = {"texDepth", texDepth};
    textures[4] = {"texBaseColorAmbient", texBaseColorAmbient};
    textures[5] = {"texEmissiveRoughness", texEmissiveRoughness};
    textures[6] = {"texNormalMetallic", texNormalMetallic};
    textures[7] = {"texSsao", texSsao};

    pipelinePbr.bindDescriptors(vkb, uniforms, textures, {});

    pipelinePbr.renderMesh(vkb, fullScreenQuad);
}

void RenderSubpassPbr::updateDirectionalLights(Scene& scene) {
    DirectionalLights uniform{};

    auto system = scene.getView<ComponentTransform, ComponentDirectionalLight>();
    for (auto&& [entity, transform, light] : system.each()) {
        uniform.colors[uniform.count] = light.getColor();
        uniform.directions[uniform.count] = Vector4{transform.getPosition(), 0.0f};

        uniform.count++;
        if (uniform.count >= sizeof(DirectionalLights::colors) / sizeof(Vector4)) {
            break;
        }
    }

    if (!directionalLightsUbo) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(DirectionalLights);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        directionalLightsUbo = vulkan.createDoubleBuffer(bufferInfo);
    }

    directionalLightsUbo.subDataLocal(&uniform, 0, sizeof(uniform));
}
