#include "ViewBuild.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"
#include "Widgets.hpp"

#define CMP "ViewVoxelTest"

using namespace Engine;

ViewBuild::ViewBuild(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                     Widgets& widgets)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), widgets(widgets) {
}

void ViewBuild::load() {
    scene = std::make_unique<Scene>();

    auto skybox = std::make_shared<Entity>();
    skybox->addComponent<ComponentSkybox>(Skybox::createOfColor(Color4{0.15f, 0.15f, 0.15f, 1.0f}));
    skybox->scale(Vector3{1000.0f});
    scene->addEntity(skybox);

    auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f} * 3.0f);
    sun->translate(Vector3{-2.0f, 2.0f, 2.0f});
    scene->addEntity(sun);

    auto camera = std::make_shared<Entity>();
    auto cmp = camera->addComponent<ComponentCameraTurntable>();
    auto userInput = camera->addComponent<ComponentUserInput>(*cmp);
    cmp->setProjection(70.0f);
    cmp->setZoom(5.0f);
    scene->addEntity(camera);
    scene->setPrimaryCamera(camera);

    auto ship = std::make_shared<Entity>();
    auto grid = ship->addComponent<ComponentGrid>();
    grid->setDirty(true);

    auto block = assetManager.find<AssetBlock>("block_hull_t1");
    /*for (auto y = 0; y < 1; y++) {
        for (auto x = -20; x < 20; x++) {
            for (auto z = 0; z < 1; z++) {
                grid->insert(Vector3i{x, y, z}, block, 0, 0, 0);
            }
        }
    }*/

    for (auto s = 0; s < 4; s++) {
        for (auto x = 0; x < 20; x++) {
            grid->insert(Vector3i{x, 0, s}, block, 0, 0, s);
        }
    }

    scene->addEntity(ship);
}

void ViewBuild::render(const Vector2i& viewport) {
    scene->update(0.1f);
    renderer.render(*scene);
}

void ViewBuild::renderGui(const Vector2i& viewport) {
    /*canvas.beginPath();
    canvas.fillColor(Color4{1.0f, 0.0f, 0.0f, 0.2f});
    canvas.rect(Vector2{50.0f}, Vector2{100.0f});
    canvas.fill();
    canvas.closePath();*/
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    scene->eventMouseMoved(pos);
}

void ViewBuild::eventMousePressed(const Vector2i& pos, MouseButton button) {
    scene->eventMousePressed(pos, button);
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    scene->eventMouseReleased(pos, button);
}

void ViewBuild::eventMouseScroll(const int xscroll, const int yscroll) {
    scene->eventMouseScroll(xscroll, yscroll);
}

void ViewBuild::eventKeyPressed(const Key key, const Modifiers modifiers) {
    scene->eventKeyPressed(key, modifiers);
}

void ViewBuild::eventKeyReleased(const Key key, const Modifiers modifiers) {
    scene->eventKeyReleased(key, modifiers);
}
