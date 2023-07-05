#include "render_subpass_skybox.hpp"
#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "render_pass_skybox.hpp"
#include "render_subpass_pbr.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassSkybox::RenderSubpassSkybox(VulkanRenderer& vulkan, RenderResources& resources,
                                         AssetsManager& assetsManager, const VulkanTexture& brdf) :
    vulkan{vulkan},
    resources{resources},
    brdf{brdf},
    defaultSkybox{vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}},
    pipelineSkybox{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("pass_skybox_vert"),
            assetsManager.getShaders().find("pass_skybox_frag"),
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
    },
    pipelinePlanet{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("pass_skybox_planet_vert"),
            assetsManager.getShaders().find("pass_skybox_planet_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<PlanetVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
            RenderPipeline::Blending::Normal,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
            RenderPipeline::Stencil::Write,
            0xff,
        },
    } {

    setAttachments({
        RenderPassSkybox::Attachments::Depth,
        RenderPassSkybox::Attachments::Forward,
    });

    addPipeline(pipelineSkybox);
    addPipeline(pipelinePlanet);
}

void RenderSubpassSkybox::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineSkybox.getDescriptorPool().reset();
    pipelinePlanet.getDescriptorPool().reset();

    updateDirectionalLights(scene);

    renderSkybox(vkb, scene);
    renderPlanets(vkb, scene);
}

void RenderSubpassSkybox::renderSkybox(VulkanCommandBuffer& vkb, Scene& scene) {
    auto camera = scene.getPrimaryCamera();
    auto skybox = scene.getSkybox();
    if (!skybox) {
        skybox = &defaultSkybox;
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

    pipelineSkybox.renderMesh(vkb, resources.getMeshSkyboxCube());
}

void RenderSubpassSkybox::renderPlanets(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelinePlanet.bind(vkb);

    auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no primary camera attached");
    }

    auto skybox = scene.getSkybox();
    if (!skybox) {
        skybox = &defaultSkybox;
    }

    const auto& entities = scene.getView<ComponentTransform, ComponentPlanet>(entt::exclude<TagDisabled>).each();
    for (auto&& [_, transform, component] : entities) {
        renderPlanet(vkb, *camera, *skybox, transform, component);
    }
}

void RenderSubpassSkybox::renderPlanet(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                       const SkyboxTextures& skybox, ComponentTransform& transform,
                                       ComponentPlanet& component) {
    if (!component.isBackground()) {
        return;
    }

    const PlanetTextures* planetTextures;

    if (component.isHighRes()) {
        if (!component.getTextures().getColor()) {
            return;
        }

        planetTextures = &component.getTextures();
    } else {
        planetTextures = &component.getPlanetType()->getLowResTextures();
    }

    std::array<UniformBindingRef, 3> uniforms{};
    uniforms[0] = {"Camera", camera.getUboZeroPos().getCurrentBuffer()};
    uniforms[1] = {"DirectionalLights", directionalLightsUbo.getCurrentBuffer()};
    uniforms[2] = {"Atmosphere", component.getPlanetType()->getUbo()};

    std::array<SamplerBindingRef, 6> textures{};
    textures[0] = {"albedoTexture", planetTextures->getColor()};
    textures[1] = {"normalTexture", planetTextures->getNormal()};
    textures[2] = {"metallicRoughnessTexture", planetTextures->getMetallicRoughness()};
    textures[3] = {"irradianceTexture", skybox.getIrradiance()};
    textures[4] = {"prefilterTexture", skybox.getPrefilter()};
    textures[5] = {"brdfTexture", brdf};

    pipelinePlanet.bindDescriptors(vkb, uniforms, textures, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

    pipelinePlanet.pushConstants(vkb,
                                 // Constants
                                 PushConstant{"modelMatrix", modelMatrix},
                                 PushConstant{"normalMatrix", normalMatrix});

    pipelinePlanet.renderMesh(vkb, resources.getMeshPlanet());
}

void RenderSubpassSkybox::updateDirectionalLights(Scene& scene) {
    RenderSubpassPbr::DirectionalLights uniform{};

    auto system = scene.getView<ComponentTransform, ComponentDirectionalLight>();
    for (auto&& [entity, transform, light] : system.each()) {
        uniform.colors[uniform.count] = light.getColor();
        uniform.directions[uniform.count] = Vector4{transform.getPosition(), 0.0f};

        uniform.count++;
        if (uniform.count >= sizeof(RenderSubpassPbr::DirectionalLights::colors) / sizeof(Vector4)) {
            break;
        }
    }

    if (!directionalLightsUbo) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(RenderSubpassPbr::DirectionalLights);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        directionalLightsUbo = vulkan.createDoubleBuffer(bufferInfo);
    }

    directionalLightsUbo.subDataLocal(&uniform, 0, sizeof(uniform));
}
