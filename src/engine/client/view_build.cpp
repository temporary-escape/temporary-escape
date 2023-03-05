#include "view_build.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

ViewBuild::ViewBuild(const Config& config, Renderer& renderer, Registry& registry, Gui& gui) :
    config{config},
    registry{registry},
    gui{gui},
    skybox{renderer.getVulkan(), Color4{0.05f, 0.05f, 0.05f, 1.0f}},
    scene{} {

    createScene();

    // auto skybox = std::make_shared<Entity>();
    // skybox->addComponent<ComponentSkybox>(Skybox::createOfColor(Color4{0.15f, 0.15f, 0.15f, 1.0f}));
    // skybox->scale(Vector3{1000.0f});
    // scene.addEntity(skybox);

    /*auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f});
    sun->translate(Vector3{3.0f, 1.0f, 3.0f});
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
    grid->setDirty(true);*/

    /*auto block = registry.getBlocks().find("block_hull_t1");
    grid->insert(Vector3i{0, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{1, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{2, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);*/

    /*block = registry.getBlocks().find("block_crew_quarters_t1");
    grid->insert(Vector3i{0, 0, 1}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{0, 0, 2}, block, 0, 0, VoxelShape::Type::Cube);
    grid->insert(Vector3i{0, 0, 3}, block, 0, 0, VoxelShape::Type::Cube);*/

    /*auto block = registry.getBlocks().find("block_crew_quarters_t1");
    for (auto a = 0; a < 4; a++) {
        for (auto b = 0; b < 4; b++) {
            grid->insert(Vector3i{a, 0, b}, block, a * b, 0, VoxelShape::Type::Cube);
        }
    }
    grid->insert(Vector3i{2, 1, 2}, block, 0, 0, VoxelShape::Type::Cube);

    entityHelperAdd = std::make_shared<Entity>();
    auto wireframe = entityHelperAdd->addComponent<ComponentWireframe>();
    wireframe->setBox(1.05f, Color4{0.25f, 1.0f, 0.25f, 1.0f});

    // entityShip->rotate(Vector3{0.0f, 1.0f, 0.0f}, 90.0f);

    scene.addEntity(entityShip);
    scene.addEntity(entityHelperAdd);
    entityHelperAdd->setVisible(false);*/
}

void ViewBuild::createScene() {
    auto sun = scene.createEntity();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f});
    sun->addComponent<ComponentTransform>().translate(Vector3{3.0f, 1.0f, 3.0f});

    auto entityCamera = scene.createEntity();
    auto& cameraTransform = entityCamera->addComponent<ComponentTransform>();
    auto& cameraCamera = entityCamera->addComponent<ComponentCamera>(cameraTransform);
    entityCamera->addComponent<ComponentUserInput>(cameraCamera);
    cameraCamera.setProjection(80.0f);
    cameraCamera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
    logger.info("Setting scene primary camera");
    scene.setPrimaryCamera(entityCamera);

    entityShip = scene.createEntity();
    auto& entityTransform = entityShip->addComponent<ComponentTransform>();
    auto& debug = entityShip->addComponent<ComponentDebug>();
    auto& grid = entityShip->addComponent<ComponentGrid>(debug);
    grid.setDirty(true);

    auto block = registry.getBlocks().find("block_crew_quarters_t1");
    grid.insert(Vector3i{0, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
}

void ViewBuild::update(const float deltaTime) {
    scene.update(deltaTime);
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);

    raycastScreenPos = pos;
}

void ViewBuild::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    scene.eventMouseReleased(pos, button);
}

void ViewBuild::eventMouseScroll(const int xscroll, const int yscroll) {
    scene.eventMouseScroll(xscroll, yscroll);
}

void ViewBuild::eventKeyPressed(const Key key, const Modifiers modifiers) {
    scene.eventKeyPressed(key, modifiers);
}

void ViewBuild::eventKeyReleased(const Key key, const Modifiers modifiers) {
    scene.eventKeyReleased(key, modifiers);
}

void ViewBuild::eventCharTyped(const uint32_t code) {
    scene.eventCharTyped(code);
}

void ViewBuild::onEnter() {
    gui.blockSelector.setEnabled(true);
}

void ViewBuild::onExit() {
    gui.blockSelector.setEnabled(false);
}

const Renderer::Options& ViewBuild::getRenderOptions() {
    static Renderer::Options options{};
    return options;
}

Scene& ViewBuild::getRenderScene() {
    return scene;
}

const Skybox& ViewBuild::getRenderSkybox() {
    return skybox;
}
