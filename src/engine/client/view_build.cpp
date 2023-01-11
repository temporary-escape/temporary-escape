#include "view_build.hpp"
#include "../graphics/renderer.hpp"

#define CMP "ViewBuild"

using namespace Engine;

ViewBuild::ViewBuild(const Config& config, Renderer& renderer, Registry& registry) :
    config{config},
    renderer{renderer},
    registry{registry},
    skybox{renderer.getVulkan(), Color4{0.03f, 0.03f, 0.03f, 1.0f}},
    scene{} {

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

void ViewBuild::update(const float deltaTime) {
    /*auto grid = entityShip->getComponent<ComponentGrid>();

    const auto [eyes, worldPos] = scene.screenToWorld(raycastScreenPos, 10.0f);
    raycastResult = grid->rayCast(eyes, worldPos);

    if (raycastResult) {
        entityHelperAdd->setVisible(true);
        entityHelperAdd->move(raycastResult->worldPos + raycastResult->normal);
    } else {
        entityHelperAdd->setVisible(false);
    }

    scene.update(deltaTime);*/
}

void ViewBuild::render(const Vector2i& viewport) {
    /*Renderer::Options options{};
    options.blurStrength = 0.2f;
    renderer.render(viewport, scene, skybox, options);*/
}

void ViewBuild::renderCanvas(const Vector2i& viewport) {
    // canvas.rect({0.0f, 0.0f}, {32.0f, 32.0f}, Color4{1.0f, 0.0f, 0.0f, 1.0f});
    // canvas.text({50.0f, 50.0f}, "Hello World! qgWQ_Ap.", font.regular, 10.5f, Color4{1.0f});

    /*nuklear.begin(viewport);
    guiBlockSelector.draw(viewport);
    nuklear.end();*/
}

void ViewBuild::renderGui(const Vector2i& viewport) {
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);

    raycastScreenPos = pos;
}

void ViewBuild::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);

    /*contextMenu.pos = camera->worldToScreen({input.hover->pos.x, 0.0f, input.hover->pos.y}, true);
        contextMenu.enabled = true;
        contextMenu.items = {
            {"View", [this]() { contextMenu.enabled = false; }},
            {"Info", [this]() { contextMenu.enabled = false; }},
        };*/
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
}

void ViewBuild::onExit() {
}
