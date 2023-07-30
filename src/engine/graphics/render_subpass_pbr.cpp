#include "render_subpass_pbr.hpp"
#include "../assets/assets_manager.hpp"
#include "../scene/controllers/controller_lights.hpp"
#include "mesh_utils.hpp"
#include "render_pass_lighting.hpp"
#include "render_pass_opaque.hpp"
#include "render_pass_shadows.hpp"
#include "render_pass_ssao.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassPbr::RenderSubpassPbr(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                   const RenderPassOpaque& opaque, const RenderPassSsao& ssao,
                                   const RenderPassShadows& shadows, const VulkanTexture& brdf) :
    vulkan{vulkan},
    resources{resources},
    opaque{opaque},
    ssao{ssao},
    brdf{brdf},
    shadows{shadows},
    defaultSkybox{vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}},
    pipelinePbr{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("pass_pbr_vert"),
            assetsManager.getShaders().find("pass_pbr_frag"),
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
}

void RenderSubpassPbr::render(VulkanCommandBuffer& vkb, Scene& scene) {
    updateDirectionalLights(scene);

    auto& controllerLights = scene.getController<ControllerLights>();

    auto camera = scene.getPrimaryCamera();
    const auto* skybox = scene.getSkybox();
    const auto* skyboxTextures{&defaultSkybox};
    if (skybox) {
        skyboxTextures = &skybox->getTextures();
    }

    pipelinePbr.getDescriptorPool().reset();

    pipelinePbr.bind(vkb);

    std::array<UniformBindingRef, 3> uniforms;
    std::array<SamplerBindingRef, 9> textures;

    const auto& texSsao = ssao.getTexture(RenderPassSsao::Attachments::Ssao);
    const auto& texDepth = opaque.getTexture(RenderPassOpaque::Attachments::Depth);
    const auto& texShadows = shadows.getShadowMapArray();
    const auto& texBaseColorAmbient = opaque.getTexture(RenderPassOpaque::Attachments::AlbedoAmbient);
    const auto& texEmissiveRoughness = opaque.getTexture(RenderPassOpaque::Attachments::EmissiveRoughness);
    const auto& texNormalMetallic = opaque.getTexture(RenderPassOpaque::Attachments::NormalMetallic);

    uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};
    uniforms[1] = {"DirectionalLights", directionalLightsUbo.getCurrentBuffer()};
    uniforms[2] = {"ShadowsViewProj", controllerLights.getUboShadowsViewProj().getCurrentBuffer()};

    textures[0] = {"texIrradiance", skyboxTextures->getIrradiance()};
    textures[1] = {"texPrefilter", skyboxTextures->getPrefilter()};
    textures[2] = {"texBrdf", brdf};
    textures[3] = {"texDepth", texDepth};
    textures[4] = {"texBaseColorAmbient", texBaseColorAmbient};
    textures[5] = {"texEmissiveRoughness", texEmissiveRoughness};
    textures[6] = {"texNormalMetallic", texNormalMetallic};
    textures[7] = {"texSsao", texSsao};
    textures[8] = {"texShadows", texShadows};

    pipelinePbr.bindDescriptors(vkb, uniforms, textures, {});

    pipelinePbr.renderMesh(vkb, resources.getMeshFullScreenQuad());
}

void RenderSubpassPbr::updateDirectionalLights(Scene& scene) {
    DirectionalLights uniform{};

    auto systemDirLights = scene.getView<ComponentTransform, ComponentDirectionalLight>();
    for (auto&& [entity, transform, light] : systemDirLights.each()) {
        if (uniform.count + 1 >= sizeof(DirectionalLights::colors) / sizeof(Vector4)) {
            break;
        }

        uniform.colors[uniform.count] = light.getColor();
        uniform.directions[uniform.count] = Vector4{transform.getPosition(), 0.0f};

        uniform.count++;
    }

    auto systemPointLights = scene.getView<ComponentTransform, ComponentPointLight>();
    for (auto&& [entity, transform, light] : systemPointLights.each()) {
        if (uniform.count + 1 >= sizeof(DirectionalLights::colors) / sizeof(Vector4)) {
            break;
        }

        uniform.colors[uniform.count] = light.getColor();
        uniform.directions[uniform.count] = Vector4{transform.getPosition(), 1.0f};

        uniform.count++;
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
