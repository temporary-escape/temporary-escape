#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Graphics/SkyboxRenderer.hpp"
#include "../Math/Vector.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Network/NetworkMessageAcceptor.hpp"
#include "../Platform/Enums.hpp"
#include "../Scene/ComponentCamera.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/EventBus.hpp"
#include "Events.hpp"
#include "Messages.hpp"
#include "ViewBuild.hpp"
#include "ViewSpace.hpp"

namespace Scissio {
struct EventBuildMode;

class SCISSIO_API Client : public Network::Client {
public:
    explicit Client(const Config& config, EventBus& eventBus, AssetManager& assetManager, Renderer& renderer,
                    SkyboxRenderer& skyboxRenderer, Canvas2D& canvas, const std::string& address);
    virtual ~Client() = default;

    void render(GBuffer& gBuffer, const Vector2i& viewport);
    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventKeyPressed(Key key);
    void eventKeyReleased(Key key);

    void dispatch(const Network::Packet& packet) override;

private:
    class Response {
    public:
        template <typename T> void store(T message) {
            {
                std::lock_guard<std::mutex> lock{mutex};
                value = std::move(message);
            }
            cv.notify_one();
        }

        template <typename T> T get() {
            std::unique_lock<std::mutex> lock{mutex};

            if (!value.has_value()) {
                const auto status = cv.wait_for(lock, std::chrono::milliseconds(2000));
                if (status == std::cv_status::timeout) {
                    throw std::runtime_error("Timeout while waiting for response");
                }
            }

            if (value.type() != typeid(T)) {
                throw std::runtime_error("Invalid response");
            }

            T res = std::any_cast<T>(value);
            value.reset();

            return res;
        }

    private:
        std::mutex mutex;
        std::any value;
        std::condition_variable cv;
    };

    void syncHello();
    void initScene();
    void handle(MessageServerHello message);
    void handle(const EventBuildMode& event);
    void handle(const EventSpaceMode& event);

    const Config& config;
    EventBus& eventBus;
    AssetManager& assetManager;
    Renderer& renderer;
    SkyboxRenderer& skyboxRenderer;
    Canvas2D& canvas;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<ViewSpace> viewSpace;
    std::unique_ptr<ViewBuild> viewBuild;
    View* view;
    Skybox skybox;
    // EntityPtr entity;

    int renderMode;

    Response response;
};
} // namespace Scissio
