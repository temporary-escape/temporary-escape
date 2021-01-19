#include "ViewSpace.hpp"

#include "../Math/Matrix.hpp"
#include "../Scene/ComponentCamera.hpp"

using namespace Scissio;

ViewSpace::ViewSpace(const Config& config, EventBus& eventBus, Renderer& renderer, Scene& scene)
    : config(config), eventBus(eventBus), renderer(renderer),
      scene(scene), cameraMove{false}, cameraRotation{0, 0}, cameraRotate{false}, entitySelected{nullptr} {
}

void ViewSpace::render(const Vector2i& viewport) {
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

    scene.render(renderer);
}

void ViewSpace::canvas(const Vector2i& viewport) {
}

void ViewSpace::eventMouseMoved(const Vector2i& pos) {
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
}

void ViewSpace::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    cameraRotate |= button == MouseButton::Right;
}

void ViewSpace::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    cameraRotate &= !(button == MouseButton::Right);
}

void ViewSpace::eventKeyPressed(const Key key) {
    cameraMove[0] |= key == Key::LetterW;
    cameraMove[1] |= key == Key::LetterA;
    cameraMove[2] |= key == Key::LetterS;
    cameraMove[3] |= key == Key::LetterD;
    cameraMove[4] |= key == Key::SpaceBar;
    cameraMove[5] |= key == Key::LeftControl;

    if (key == Key::LetterB) {
        eventBus.publish(EventBuildMode{nullptr});
    }
}

void ViewSpace::eventKeyReleased(const Key key) {
    cameraMove[0] &= !(key == Key::LetterW);
    cameraMove[1] &= !(key == Key::LetterA);
    cameraMove[2] &= !(key == Key::LetterS);
    cameraMove[3] &= !(key == Key::LetterD);
    cameraMove[4] &= !(key == Key::SpaceBar);
    cameraMove[5] &= !(key == Key::LeftControl);
}
