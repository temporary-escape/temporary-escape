#include "render_subpass_shadows.hpp"
#include "../assets/assets_manager.hpp"
#include "../scene/controllers/controller_lights.hpp"
#include "../scene/controllers/controller_static_model.hpp"
#include "mesh_utils.hpp"
#include "render_pass_outline.hpp"
#include "theme.hpp"

using namespace Engine;

RenderSubpassShadows::RenderSubpassShadows(VulkanRenderer& vulkan, RenderResources& resources,
                                           AssetsManager& assetsManager, const size_t index) :
    vulkan{vulkan},
    resources{resources},
    index{index},
    pipelineGrid{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_grid_vert"),
            assetsManager.getShaders().find("component_shadow_frag"),
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
            assetsManager.getShaders().find("component_shadow_frag"),
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
            assetsManager.getShaders().find("component_shadow_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0),
            RenderPipeline::VertexInput::of<ComponentModel::InstancedVertex>(
                1, VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE),
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
        RenderPassOutline::Attachments::Color,
    });

    addPipeline(pipelineGrid);
    addPipeline(pipelineModel);
    addPipeline(pipelineModelInstanced);
}

void RenderSubpassShadows::render(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();

    if (!controllerLights.getUboShadowCamera()) {
        return;
    }

    pipelineGrid.getDescriptorPool().reset();
    pipelineModel.getDescriptorPool().reset();
    pipelineModelInstanced.getDescriptorPool().reset();

    renderSceneGrids(vkb, scene);
    renderSceneModels(vkb, scene);
    renderSceneModelsStatic(vkb, scene);
}

void RenderSubpassShadows::renderSceneGrids(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();
    auto systemGrids = scene.getView<ComponentTransform, ComponentGrid>();

    std::array<UniformBindingRef, 1> uniforms;
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    pipelineGrid.bind(vkb);

    for (auto&& [entity, transform, grid] : systemGrids.each()) {
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

            vboBindings[0] = {&primitive.vbo, 0};
            vkb.bindBuffers(vboBindings);

            vkb.bindIndexBuffer(primitive.ibo, 0, primitive.indexType);

            uniforms[0] = {"Camera",
                           controllerLights.getUboShadowCamera().getCurrentBuffer(),
                           index * sizeof(Camera::Uniform),
                           sizeof(Camera::Uniform)};

            pipelineGrid.bindDescriptors(vkb, uniforms, {}, {});

            vkb.drawIndexed(primitive.count, 1, 0, 0, 0);
        }
    }
}

void RenderSubpassShadows::renderSceneModels(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();
    auto systemModels = scene.getView<ComponentTransform, ComponentModel>();

    std::array<UniformBindingRef, 1> uniforms;
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    pipelineModel.bind(vkb);

    for (auto&& [entity, transform, model] : systemModels.each()) {
        if (transform.isStatic()) {
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

            vboBindings[0] = {&primitive.vbo, 0};
            vkb.bindBuffers(vboBindings);

            vkb.bindIndexBuffer(primitive.ibo, 0, primitive.indexType);

            uniforms[0] = {"Camera",
                           controllerLights.getUboShadowCamera().getCurrentBuffer(),
                           index * sizeof(Camera::Uniform),
                           sizeof(Camera::Uniform)};

            pipelineModel.bindDescriptors(vkb, uniforms, {}, {});

            vkb.drawIndexed(primitive.count, 1, 0, 0, 0);
        }
    }
}

void RenderSubpassShadows::renderSceneModelsStatic(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();
    auto& controllerStaticModel = scene.getController<ControllerStaticModel>();

    pipelineModelInstanced.bind(vkb);

    std::array<UniformBindingRef, 1> uniforms;
    std::array<VulkanVertexBufferBindRef, 2> vboBindings{};

    for (auto&& [model, buffer] : controllerStaticModel.getBuffers()) {
        for (auto& primitive : model->getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            vboBindings[0] = {&primitive.vbo, 0};
            vboBindings[1] = {&buffer.getCurrentBuffer(), 0};
            vkb.bindBuffers(vboBindings);

            vkb.bindIndexBuffer(primitive.ibo, 0, primitive.indexType);

            uniforms[0] = {"Camera",
                           controllerLights.getUboShadowCamera().getCurrentBuffer(),
                           index * sizeof(Camera::Uniform),
                           sizeof(Camera::Uniform)};

            pipelineModelInstanced.bindDescriptors(vkb, uniforms, {}, {});

            vkb.drawIndexed(primitive.count, buffer.count(), 0, 0, 0);
        }
    }
}
