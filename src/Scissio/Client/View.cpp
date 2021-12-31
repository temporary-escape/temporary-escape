#include "View.hpp"
#include "Client.hpp"

#define CMP "View"

using namespace Scissio;

View::View(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer, Client& client)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), client(client),
      gui(canvas, config, assetManager), widgets{WidgetDebugStats(gui, client.getStats())} {
}

void View::render(const Vector2i& viewport) {
    if (auto scene = client.getScene(); scene != nullptr) {
        auto& componentSystemCamera = scene->getComponentSystem<ComponentCamera>();
        auto front = componentSystemCamera.begin();
        if (front != componentSystemCamera.end()) {
            auto camera = *front;

            static constexpr auto d = 0.1f;

            Vector3 dir{0.0f};
            if (cameraData.move[0]) {
                dir += Vector3{0.0f, 0.0f, -d};
            }
            if (cameraData.move[1]) {
                dir += Vector3{-d, 0.0f, 0.0f};
            }
            if (cameraData.move[2]) {
                dir += Vector3{0.0f, 0.0f, d};
            }
            if (cameraData.move[3]) {
                dir += Vector3{d, 0.0f, 0.0f};
            }
            if (cameraData.move[4]) {
                dir += Vector3{0.0f, d, 0.0f};
            }
            if (cameraData.move[5]) {
                dir += Vector3{0.0f, -d, 0.0f};
            }

            const auto translation = Vector3(camera->getObject().getTransform()[3]);

            glm::mat4x4 transform{1.0f};
            transform = glm::rotate(transform, glm::radians(cameraData.rotation.x), Vector3{0.0f, 1.0f, 0.0f});
            transform = glm::rotate(transform, glm::radians(cameraData.rotation.y), Vector3{1.0f, 0.0f, 0.0f});

            dir = Vector3(transform * Vector4(dir, 1.0f));

            transform = glm::translate(glm::mat4x4{1.0f}, translation + dir) * transform;
            camera->getObject().updateTransform(transform);
        }
    }

    if (auto scene = client.getScene(); scene != nullptr) {
        renderer.render(viewport, *scene);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    canvas.beginFrame(viewport);

    /*canvas.beginPath();
    canvas.strokeColor(Color4{1.0f, 0.0f, 0.0f, 1.0f});
    canvas.strokeWidth(2.0f);
    canvas.moveTo(Vector2{0.0f, 50.0f});
    for (auto i = 0; i < 500; i++) {
        canvas.lineTo(Vector2{i * 2.0f, 50.0f});
    }
    canvas.stroke();
    canvas.closePath();*/

    gui.reset();

    // widgets.debugStats.render();

    gui.render(viewport);
    canvas.endFrame();
}

void View::eventMouseMoved(const Vector2i& pos) {
    if (cameraData.rotate) {
        cameraData.rotation += (cameraData.mousePosOld - Vector2(pos)) * 0.2f;
        while (cameraData.rotation.x > 360.0f) {
            cameraData.rotation.x -= 360.0f;
        }
        while (cameraData.rotation.x < 0.0f) {
            cameraData.rotation.x += 360.0f;
        }
        cameraData.rotation.y = glm::clamp(cameraData.rotation.y, -90.0f, 90.0f);
        cameraData.mousePosOld = pos;
    }
}

void View::eventMousePressed(const Vector2i& pos, MouseButton button) {
    cameraData.mousePosOld = pos;
    cameraData.rotate |= button == MouseButton::Right;
}

void View::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    cameraData.mousePosOld = pos;
    cameraData.rotate &= !(button == MouseButton::Right);
}

void View::eventMouseScroll(int xscroll, int yscroll) {
}

void View::eventKeyPressed(Key key, Modifiers modifiers) {
    cameraData.move[0] |= key == Key::LetterW;
    cameraData.move[1] |= key == Key::LetterA;
    cameraData.move[2] |= key == Key::LetterS;
    cameraData.move[3] |= key == Key::LetterD;
    cameraData.move[4] |= key == Key::SpaceBar;
    cameraData.move[5] |= key == Key::LeftControl;

    if (key == Key::LetterM) {
        MessageFetchCurrentLocation reqLoc;
        client.fetch(reqLoc, [this](PlayerLocationData location) {
            MessageFetchGalaxySystems reqSystems;
            reqSystems.galaxyId = location.galaxyId;
            client.fetch(reqSystems, [this](std::vector<SystemData> items) {
                Log::i(CMP, "Received {} SystemData items", items.size());
            });
        });

        /*MessageFetchGalaxySystems req;
        req.galaxyId = "aabbcc";
        client.fetch(
            req, [this](std::vector<SystemData> items) { Log::i(CMP, "Received {} SystemData items", items.size());
        });*/
    }
}

void View::eventKeyReleased(Key key, Modifiers modifiers) {
    cameraData.move[0] &= !(key == Key::LetterW);
    cameraData.move[1] &= !(key == Key::LetterA);
    cameraData.move[2] &= !(key == Key::LetterS);
    cameraData.move[3] &= !(key == Key::LetterD);
    cameraData.move[4] &= !(key == Key::SpaceBar);
    cameraData.move[5] &= !(key == Key::LeftControl);
}
