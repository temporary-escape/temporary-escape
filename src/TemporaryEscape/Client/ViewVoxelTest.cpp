#include "ViewVoxelTest.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"
#include "Widgets.hpp"

using namespace Engine;

ViewVoxelTest::ViewVoxelTest(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                             Widgets& widgets)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), widgets(widgets) {

    scene = std::make_unique<Scene>();

    auto skybox = std::make_shared<Entity>();
    skybox->addComponent<ComponentSkybox>(12345678);
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

    auto asteroidModel = assetManager.find<AssetModel>("model_asteroid_01_a");

    for (auto y = 0; y < 10; y++) {
        for (auto x = 0; x < 10; x++) {
            auto asteroid = std::make_shared<Entity>();
            asteroid->addComponent<ComponentModel>(asteroidModel);
            asteroid->translate(Vector3{x * 3.0f, -3.0f, y * 3.0f});
            scene->addEntity(asteroid);
        }
    }
}

void ViewVoxelTest::render(const Vector2i& viewport) {
    if (scene != nullptr) {
        scene->update(0.1f);
        renderer.render(*scene);
    }
}

void ViewVoxelTest::renderGui(const Vector2i& viewport) {
}

void ViewVoxelTest::eventMouseMoved(const Vector2i& pos) {
    if (scene) {
        scene->eventMouseMoved(pos);
    }
}

void ViewVoxelTest::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (scene) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewVoxelTest::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (scene) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewVoxelTest::eventMouseScroll(const int xscroll, const int yscroll) {
    if (scene) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewVoxelTest::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (scene) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewVoxelTest::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (scene) {
        scene->eventKeyReleased(key, modifiers);
    }
}
