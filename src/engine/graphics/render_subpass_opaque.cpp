#include "render_subpass_opaque.hpp"
#include "../assets/assets_manager.hpp"
#include "../scene/components/component_grid.hpp"
#include "../scene/controllers/controller_model.hpp"
#include "mesh_utils.hpp"
#include "render_pass_opaque.hpp"

using namespace Engine;

RenderSubpassOpaque::RenderSubpassOpaque(VulkanRenderer& vulkan, RenderResources& resources,
                                         AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache) :
    vulkan{vulkan},
    resources{resources},
    voxelShapeCache{voxelShapeCache},
    pipelineGrid{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_grid_vert"),
            assetsManager.getShaders().find("component_grid_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<VoxelShape::VertexFinal>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    },
    pipelineModel{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_model_vert"),
            assetsManager.getShaders().find("component_model_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    },
    pipelineModelInstanced{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_model_instanced_vert"),
            assetsManager.getShaders().find("component_model_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0),
            RenderPipeline::VertexInput{
                2,
                {
                    {4, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 0},
                    {5, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 1},
                    {6, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 2},
                    {7, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 3},
                    {8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 4},
                },
                sizeof(Vector4) + sizeof(Matrix4),
                VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE,
            },
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
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
            assetsManager.getShaders().find("component_planet_vert"),
            assetsManager.getShaders().find("component_planet_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<PlanetVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassOpaque::Attachments::Depth,
        RenderPassOpaque::Attachments::AlbedoAmbient,
        RenderPassOpaque::Attachments::EmissiveRoughness,
        RenderPassOpaque::Attachments::NormalMetallic,
        RenderPassOpaque::Attachments::Entity,
    });

    addPipeline(pipelineGrid);
    addPipeline(pipelineModel);
    addPipeline(pipelineModelInstanced);
    addPipeline(pipelinePlanet);

    palette = assetsManager.getTextures().find("palette");
}

void RenderSubpassOpaque::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineGrid.getDescriptorPool().reset();
    pipelineModel.getDescriptorPool().reset();
    pipelineModelInstanced.getDescriptorPool().reset();
    pipelinePlanet.getDescriptorPool().reset();

    renderSceneGrids(vkb, scene);
    renderSceneModels(vkb, scene);
    renderSceneModelsStatic(vkb, scene);
    renderScenePlanets(vkb, scene);
}

static void validateMaterial(const Material& material) {
    if (!material.ubo) {
        EXCEPTION("Primitive has no material uniform buffer allocated");
    }

    if (!material.baseColorTexture || !material.baseColorTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no base color texture");
    }

    if (!material.emissiveTexture || !material.emissiveTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no emissive texture");
    }

    if (!material.normalTexture || !material.normalTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no normal texture");
    }

    if (!material.ambientOcclusionTexture || !material.ambientOcclusionTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no ambient occlusion texture");
    }

    if (!material.metallicRoughnessTexture || !material.metallicRoughnessTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no metallic roughness texture");
    }
}

void RenderSubpassOpaque::renderSceneGrids(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemGrids = scene.getView<ComponentTransform, ComponentGrid>();
    auto camera = scene.getPrimaryCamera();

    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 6> textures;
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    pipelineGrid.bind(vkb);

    for (auto&& [entity, transform, grid] : systemGrids.each()) {
        grid.recalculate(vulkan, voxelShapeCache);

        const auto modelMatrix = transform.getAbsoluteTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineGrid.pushConstants(vkb,
                                   // Constants
                                   PushConstant{"modelMatrix", modelMatrix},
                                   PushConstant{"normalMatrix", normalMatrix},
                                   PushConstant{"entityColor", entityColor(entity)});

        for (auto& primitive : grid.getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            validateMaterial(*primitive.material);

            vboBindings[0] = {&primitive.vbo, 0};
            vkb.bindBuffers(vboBindings);

            vkb.bindIndexBuffer(primitive.ibo, 0, primitive.indexType);

            uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};
            uniforms[1] = {"Material", primitive.material->ubo};

            textures[0] = {"baseColorTexture", primitive.material->baseColorTexture->getVulkanTexture()};
            textures[1] = {"emissiveTexture", primitive.material->emissiveTexture->getVulkanTexture()};
            textures[2] = {"normalTexture", primitive.material->normalTexture->getVulkanTexture()};
            textures[3] = {"ambientOcclusionTexture", primitive.material->ambientOcclusionTexture->getVulkanTexture()};
            textures[4] = {"metallicRoughnessTexture",
                           primitive.material->metallicRoughnessTexture->getVulkanTexture()};
            textures[5] = {"paletteTexture", palette->getVulkanTexture()};

            pipelineGrid.bindDescriptors(vkb, uniforms, textures, {});

            vkb.drawIndexed(primitive.count, 1, 0, 0, 0);
        }
    }
}

void RenderSubpassOpaque::renderSceneModels(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemModels = scene.getView<ComponentTransform, ComponentModel>();
    auto camera = scene.getPrimaryCamera();

    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 5> textures;
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    pipelineModel.bind(vkb);

    for (auto&& [entity, transform, model] : systemModels.each()) {
        if (model.isStatic()) {
            continue;
        }

        const auto modelMatrix = transform.getAbsoluteTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineModel.pushConstants(vkb,
                                    // Constants
                                    PushConstant{"modelMatrix", modelMatrix},
                                    PushConstant{"normalMatrix", normalMatrix},
                                    PushConstant{"entityColor", entityColor(entity)});

        for (auto& primitive : model.getModel()->getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            validateMaterial(*primitive.material);

            vboBindings[0] = {&primitive.vbo, 0};
            vkb.bindBuffers(vboBindings);

            vkb.bindIndexBuffer(primitive.ibo, 0, primitive.indexType);

            uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};
            uniforms[1] = {"Material", primitive.material->ubo};

            textures[0] = {"baseColorTexture", primitive.material->baseColorTexture->getVulkanTexture()};
            textures[1] = {"emissiveTexture", primitive.material->emissiveTexture->getVulkanTexture()};
            textures[2] = {"normalTexture", primitive.material->normalTexture->getVulkanTexture()};
            textures[3] = {"ambientOcclusionTexture", primitive.material->ambientOcclusionTexture->getVulkanTexture()};
            textures[4] = {"metallicRoughnessTexture",
                           primitive.material->metallicRoughnessTexture->getVulkanTexture()};

            pipelineModel.bindDescriptors(vkb, uniforms, textures, {});

            vkb.drawIndexed(primitive.count, 1, 0, 0, 0);
        }
    }
}

void RenderSubpassOpaque::renderSceneModelsStatic(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerModel = scene.getController<ControllerModel>();
    controllerModel.recalculate(vulkan);

    auto camera = scene.getPrimaryCamera();

    pipelineModelInstanced.bind(vkb);

    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 5> textures;
    std::array<VulkanVertexBufferBindRef, 2> vboBindings{};

    /*for (auto&& [model, buffer] : controllerModel.getBuffers()) {
        for (auto& primitive : model->getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            validateMaterial(*primitive.material);

            vboBindings[0] = {&primitive.vbo, 0};
            vboBindings[1] = {&buffer.vboEntityColors.getCurrentBuffer(), 0};
            vboBindings[2] = {&buffer.vboModels.getCurrentBuffer(), 0};
            vkb.bindBuffers(vboBindings);

            vkb.bindIndexBuffer(primitive.ibo, 0, primitive.indexType);

            uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};
            uniforms[1] = {"Material", primitive.material->ubo};

            textures[0] = {"baseColorTexture", primitive.material->baseColorTexture->getVulkanTexture()};
            textures[1] = {"emissiveTexture", primitive.material->emissiveTexture->getVulkanTexture()};
            textures[2] = {"normalTexture", primitive.material->normalTexture->getVulkanTexture()};
            textures[3] = {"ambientOcclusionTexture", primitive.material->ambientOcclusionTexture->getVulkanTexture()};
            textures[4] = {"metallicRoughnessTexture",
                           primitive.material->metallicRoughnessTexture->getVulkanTexture()};

            pipelineModelInstanced.bindDescriptors(vkb, uniforms, textures, {});

            vkb.drawIndexed(primitive.count, buffer.count, 0, 0, 0);
        }
    }*/
}

void RenderSubpassOpaque::renderScenePlanets(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemPlanets = scene.getView<ComponentTransform, ComponentPlanet>();
    auto camera = scene.getPrimaryCamera();

    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 3> textures;
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    pipelinePlanet.bind(vkb);

    for (auto&& [entity, transform, planet] : systemPlanets.each()) {
        if (planet.isBackground()) {
            continue;
        }

        uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};
        uniforms[1] = {"Atmosphere", planet.getPlanetType()->getUbo()};

        const auto& planetTextures = planet.getPlanetType()->getLowResTextures();

        textures[0] = {"albedoTexture", planetTextures.getColor()};
        textures[1] = {"normalTexture", planetTextures.getNormal()};
        textures[2] = {"metallicRoughnessTexture", planetTextures.getMetallicRoughness()};

        pipelinePlanet.bindDescriptors(vkb, uniforms, textures, {});

        const auto modelMatrix = transform.getAbsoluteTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelinePlanet.pushConstants(vkb,
                                     // Constants
                                     PushConstant{"modelMatrix", modelMatrix},
                                     PushConstant{"normalMatrix", normalMatrix});

        pipelinePlanet.renderMesh(vkb, resources.getMeshPlanet());
    }
}
