#include "ViewBuild.hpp"

#include "../Assets/Model.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Utils.hpp"
#include "../Scene/ComponentCamera.hpp"
#include "../Scene/ComponentGrid.hpp"
#include "../Scene/ComponentModel.hpp"

#include <iostream>

using namespace Scissio;

ViewBuild::ViewBuild(const Config& config, EventBus& eventBus, AssetManager& assetManager, Renderer& renderer,
                     Canvas2D& canvas)
    : config(config), eventBus(eventBus), assetManager(assetManager), renderer(renderer),
      gui(canvas, config, assetManager), cameraMove{false}, cameraRotation{0, 0}, cameraRotate{false},
      blockSelector(gui, assetManager), menu(gui, assetManager) {

    /*auto model = assetManager.find<Model>("model_engine_01");
    auto entity = scene.addEntity();
    entity->addComponent<ComponentModel>(model);
    entity->translate({0.0f, 0.0f, 0.0f});
    // entity->rotate({ 0.0f, 1.0f, 0.0f }, 90.0f);

    model = assetManager.find<Model>("SciFiHelmet");
    entity = scene.addEntity();
    entity->addComponent<ComponentModel>(model);
    entity->translate({0.0f, 0.0f, -2.0f});*/

    auto entity = scene.addEntity();
    camera = entity->addComponent<ComponentCamera>();
    entity->translate({0.0f, 0.0f, 2.0f});

    ship = scene.addEntity();
    auto block = assetManager.find<Block>("block_hull_01_cube");
    auto grid = ship->addComponent<ComponentGrid>();
    grid->insert(block, {0, 0, 0}, 0);
    grid->insert(block, {1, 0, 0}, 0);
    grid->insert(block, {2, 0, 0}, 0);
    ship->translate({0.0f, 0.0f, 0.0f});

    preview = scene.addEntity();
    preview->addComponent<ComponentModel>(block->getModel());
    preview->translate({0.0f, -1000.0f, 0.0f});
}

void ViewBuild::render(const Vector2i& viewport) {
    this->viewport = viewport;

    auto& systemCamera = scene.getComponentSystem<ComponentCamera>();

    for (auto& camera : systemCamera) {
        if (camera->isActive()) {
            static constexpr auto d = 0.1f;

            Vector3 dir{0.0f};
            if (cameraMove[0]) {
                dir += Vector3{0.0f, 0.0f, -d};
            }
            if (cameraMove[1]) {
                dir += Vector3{-d, 0.0f, 0.0f};
            }
            if (cameraMove[2]) {
                dir += Vector3{0.0f, 0.0f, d};
            }
            if (cameraMove[3]) {
                dir += Vector3{d, 0.0f, 0.0f};
            }
            if (cameraMove[4]) {
                dir += Vector3{0.0f, d, 0.0f};
            }
            if (cameraMove[5]) {
                dir += Vector3{0.0f, -d, 0.0f};
            }

            const auto translation = Vector3(camera->getObject().getTransform()[3]);

            glm::mat4x4 transform{1.0f};
            transform = glm::rotate(transform, glm::radians(cameraRotation.x), Vector3{0.0f, 1.0f, 0.0f});
            transform = glm::rotate(transform, glm::radians(cameraRotation.y), Vector3{1.0f, 0.0f, 0.0f});

            dir = Vector3(transform * Vector4(dir, 1.0f));

            transform = glm::translate(glm::mat4x4{1.0f}, translation + dir) * transform;
            camera->getObject().getTransform() = transform;
            camera->setProjection(viewport, 70.0f);

            renderer.setCamera(camera->getViewMatrix());
            renderer.setProjection(camera->getProjectionMatrix());

            break;
        }
    }

    {
        const auto to = camera->getEyesPos() + camera->screenToWorld(viewport, mousePos) * 100.0f;
        const auto result = ship->getComponent<ComponentGrid>()->rayCast(camera->getEyesPos(), to);

        if (result.has_value()) {
            const auto& r = result.value();
            preview->move(r.node.get().pos + r.normal);
        } else {
            preview->move(Vector3{-99999.0f});
        }
    }

    // const auto projected = camera->getEyesPos() + camera->screenToWorld(viewport, mousePos) * 10.0f;
    // preview->move(projected);

    scene.render(renderer);
}

void ViewBuild::canvas(const Vector2i& viewport) {
    gui.render(viewport);
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    this->mousePos = pos;

    if (cameraRotate) {
        cameraRotation += (mousePosOld - Vector2(pos)) * 0.2f;
        while (cameraRotation.x > 360.0f) {
            cameraRotation.x -= 360.0f;
        }
        while (cameraRotation.x < 0.0f) {
            cameraRotation.x += 360.0f;
        }
        cameraRotation.y = glm::clamp(cameraRotation.y, -90.0f, 90.0f);
        mousePosOld = pos;
    }

    gui.mouseMoveEvent(pos);
}

void ViewBuild::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    cameraRotate |= button == MouseButton::Right;

    gui.mousePressEvent(pos, button);

    /*if (button == MouseButton::Left) {
        const auto to = camera->getEyesPos() + camera->screenToWorld(viewport, mousePos) * 100.0f;
        const auto result =
            ship->getComponent<ComponentGrid>()->rayCast(camera->getEyesPos(), to, ship->getTransform());
        if (result.has_value()) {
            const auto& r = result.value();
            std::cout << "has result: " << r.pos << " normal: " << r.normal << " side: " << r.side << std::endl;
        } else {
            std::cout << "no result" << std::endl;
        }
    }*/
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    cameraRotate &= !(button == MouseButton::Right);

    gui.mouseReleaseEvent(pos, button);
}

void ViewBuild::eventKeyPressed(const Key key) {
    cameraMove[0] |= key == Key::LetterW;
    cameraMove[1] |= key == Key::LetterA;
    cameraMove[2] |= key == Key::LetterS;
    cameraMove[3] |= key == Key::LetterD;
    cameraMove[4] |= key == Key::SpaceBar;
    cameraMove[5] |= key == Key::LeftControl;

    if (key == Key::LetterB) {
        eventBus.publish(EventSpaceMode{});
    }

    if (key == Key::LetterH) {
        blockSelector.setHidden(!blockSelector.isHidden());
    }

    gui.keyPressEvent(key);
}

void ViewBuild::eventKeyReleased(const Key key) {
    cameraMove[0] &= !(key == Key::LetterW);
    cameraMove[1] &= !(key == Key::LetterA);
    cameraMove[2] &= !(key == Key::LetterS);
    cameraMove[3] &= !(key == Key::LetterD);
    cameraMove[4] &= !(key == Key::SpaceBar);
    cameraMove[5] &= !(key == Key::LeftControl);

    gui.keyReleaseEvent(key);
}
