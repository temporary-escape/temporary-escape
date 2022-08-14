#include "ViewBuild.hpp"

#define CMP "ViewBuild"

using namespace Engine;

ViewBuild::ViewBuild(const Config& config, VulkanDevice& vulkan, Scene::Pipelines& scenePipelines, Registry& registry,
                     Canvas& canvas, Nuklear& nuklear) :
    config{config},
    vulkan{vulkan},
    scenePipelines{scenePipelines},
    registry{registry},
    canvas{canvas},
    nuklear{nuklear},
    skybox{vulkan, Color4{0.3f, 0.3f, 0.3f, 1.0f}} {

    // auto skybox = std::make_shared<Entity>();
    // skybox->addComponent<ComponentSkybox>(Skybox::createOfColor(Color4{0.15f, 0.15f, 0.15f, 1.0f}));
    // skybox->scale(Vector3{1000.0f});
    // scene.addEntity(skybox);

    auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f});
    sun->translate(Vector3{3.0f, 3.0f, 1.0f});
    scene.addEntity(sun);

    auto camera = std::make_shared<Entity>();
    auto cmp = camera->addComponent<ComponentCamera>();
    camera->addComponent<ComponentUserInput>(*cmp);
    cmp->setProjection(80.0f);
    cmp->lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
    // cmp->setZoom(5.0f);
    scene.addEntity(camera);
    scene.setPrimaryCamera(camera);

    entityShip = std::make_shared<Entity>();
    auto debug = entityShip->addComponent<ComponentDebug>();
    auto grid = entityShip->addComponent<ComponentGrid>(debug.get());
    grid->setDirty(true);

    auto block = registry.getBlocks().find("block_hull_t1");
    grid->insert(Vector3i{0, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{1, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{2, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);

    block = registry.getBlocks().find("block_crew_quarters_t1");
    grid->insert(Vector3i{0, 0, 1}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{0, 0, 2}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{0, 0, 3}, block, 0, 0, VoxelShape::Type::Cube);

    entityHelperAdd = std::make_shared<Entity>();
    auto wireframe = entityHelperAdd->addComponent<ComponentWireframe>();
    wireframe->setBox(1.05f, Color4{0.25f, 1.0f, 0.25f, 1.0f});

    scene.addEntity(entityShip);
    scene.addEntity(entityHelperAdd);
    entityHelperAdd->setVisible(false);
}

void ViewBuild::update(float deltaTime) {
    auto grid = entityShip->getComponent<ComponentGrid>();

    const auto [eyes, worldPos] = scene.screenToWorld(raycastScreenPos, 10.0f);
    raycastResult = grid->rayCast(eyes, worldPos);

    if (raycastResult) {
        entityHelperAdd->setVisible(true);
        entityHelperAdd->move(raycastResult->worldPos + raycastResult->normal);
    } else {
        entityHelperAdd->setVisible(false);
    }

    scene.update(deltaTime);
}

void ViewBuild::renderPbr(const Vector2i& viewport) {
    scene.renderPbr(vulkan, viewport, scenePipelines, registry.getVoxelShapeCache());
}

void ViewBuild::renderFwd(const Vector2i& viewport) {
    scene.renderFwd(vulkan, viewport, scenePipelines);
}

void ViewBuild::renderCanvas(const Vector2i& viewport) {
    canvas.rect({0.0f, 0.0f}, {32.0f, 32.0f}, Color4{1.0f, 0.0f, 0.0f, 1.0f});
}

const Skybox* ViewBuild::getSkybox() {
    return &skybox;
}

void ViewBuild::eventUserInput(const UserInput::Event& event) {
    scene.eventUserInput(event);

    if (event.type == Input::PointerMovement) {
        raycastScreenPos = event.value;
    }
}
