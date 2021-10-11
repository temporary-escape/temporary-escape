#include "Client.hpp"

#include "../Assets/Model.hpp"
#include "../Network/NetworkTcpConnector.hpp"
#include "../Scene/ComponentGrid.hpp"
#include "../Scene/ComponentModel.hpp"
#include "Widgets.hpp"
#include <fstream>

using namespace Scissio;

#define DISPATCH_FUNC(M, T, F) std::bind(static_cast<void (T::*)(M)>(&T::F), this, std::placeholders::_1)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Client, handle));
#define WORK(...)                                                                                                      \
    try {                                                                                                              \
        worker.post(__VA_ARGS__);                                                                                      \
    } catch (std::exception & e) {                                                                                     \
        BACKTRACE(e, "async work error");                                                                              \
    }

Client::Client(const Config& config, AssetManager& assetManager, Renderer& renderer, SkyboxRenderer& skyboxRenderer,
               ThumbnailRenderer& thumbnailRenderer, Canvas2D& canvas, const std::string& address, const uint64_t uid)
    : AbstractClient(config, assetManager, address, uid), renderer(renderer), skyboxRenderer(skyboxRenderer),
      thumbnailRenderer(thumbnailRenderer), canvas(canvas), gui(canvas, config, assetManager),
      view(nullptr), mousePos{-1} {
}

#undef MESSAGE_DISPATCH
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, AbstractClient, handle));

AbstractClient::AbstractClient(const Config& config, AssetManager& assetManager, const std::string& address,
                               const uint64_t uid)
    : Network::TcpClient(address, config.serverPort), config(config), assetManager(assetManager) {

    // MESSAGE_DISPATCH(MessageHelloResponse);
    MESSAGE_DISPATCH(MessageSectorChanged);
    MESSAGE_DISPATCH(MessageSectorStatusResponse);
    MESSAGE_DISPATCH(MessageEntityBatch);
    MESSAGE_DISPATCH(MessageSystemsResponse);
    MESSAGE_DISPATCH(MessageRegionsResponse);
    MESSAGE_DISPATCH(MessageBlocksResponse);
}

void AbstractClient::update() {
    worker.run();
    if (scene) {
        scene->update();
    }
}

void Client::render(GBuffer& gBuffer, FBuffer& fbuffer, const Vector2i& viewport) {
    if (scene && view && skybox.texture) {
        view->update(viewport);

        renderer.setViewport(viewport);
        renderer.setGBuffer(gBuffer);
        renderer.setFBuffer(fbuffer);
        renderer.setSkybox(skybox);
        renderer.setQueryPos(mousePos);
        view->render(viewport, renderer);
        fbuffer.blit(Framebuffer::DefaultFramebuffer);

        canvas.beginFrame(viewport);
        gui.reset();
        view->renderCanvas(viewport, canvas, gui);
        gui.render(viewport);
        canvas.endFrame();
    } else {
        canvas.beginFrame(viewport);
        gui.reset();

        if (scene) {
            Widgets::modal(gui, "Connecting", "Loading sector data...");
        } else {
            Widgets::modal(gui, "Connecting", "Entering sector...");
        }

        gui.render(viewport);
        canvas.endFrame();
    }
}

void AbstractClient::handle(MessageSystemsResponse message) {
    WORK([message{std::move(message)}, this]() {
        Log::d("Received {} systems", message.results.size());
        if (!message.results.empty()) {
            store.systems.value().insert(store.systems.value().end(), message.results.begin(), message.results.end());

            MessageSystemsRequest req{message.cont};
            send(req);
        } else {
            store.systems.notify();
        }
    });
}

void AbstractClient::handle(MessageRegionsResponse message) {
    WORK([message{std::move(message)}, this]() {
        Log::d("Received {} regions", message.results.size());
        if (!message.results.empty()) {
            store.regions.value().insert(store.regions.value().end(), message.results.begin(), message.results.end());

            MessageRegionsRequest req{message.cont};
            send(req);
        } else {
            store.regions.notify();
        }
    });
}

void AbstractClient::handle(MessageBlocksResponse message) {
    WORK([message{std::move(message)}, this]() {
        Log::d("Received {} blocks", message.results.size());
        if (!message.results.empty()) {
            store.blocks.value().insert(store.blocks.value().end(), message.results.begin(), message.results.end());

            for (const auto& block : message.results) {
                onBlockReceived(block);
            }

            MessageBlocksRequest req{message.cont};
            send(req);
        } else {
            store.blocks.notify();
        }
    });
}

void AbstractClient::handle(MessageHelloResponse message) {
    (void)message;
}

void AbstractClient::handle(MessageLoginResponse message) {
    (void)message;
}

void AbstractClient::handle(MessageSectorChanged message) {
    WORK([message{std::move(message)}, this]() {
        store.sector.value() = message.sector;
        store.sector.notify();

        scene = std::make_unique<Scene>();

        send(MessageSectorStatusRequest{});
    });
}

void AbstractClient::handle(MessageSectorStatusResponse message) {
    WORK([message, this]() {
        if (!message.loaded) {
            send(MessageSectorStatusRequest{});
        } else {
            onSectorLoaded();
        }
    });
}

void Client::onSectorLoaded() {
    WORK([this]() {
        viewSpace = std::make_unique<ViewSpace>(config, *this, *scene);
        view = viewSpace.get();

        skybox = skyboxRenderer.renderAndFilter(7418525);
    });
}

void Client::onBlockReceived(const BlockDto& block) {
    WORK([block, this]() {
        const auto thumbnailName = fmt::format("{}_thumb", block.key);
        auto thumbnail = assetManager.findOrNull<Image>(thumbnailName);

        if (!thumbnail) {
            try {
                thumbnailRenderer.render(renderer, block.model);
                auto pixels = thumbnailRenderer.getPixels();
                auto size = thumbnailRenderer.getSize();

                std::ofstream file(block.key + ".raw", std::ios::binary);
                file.write(pixels.get(), size.x * size.y * 4);

                thumbnail = assetManager.generateImage(block.model->getMod(), thumbnailName, size, std::move(pixels));

                store.thumbnails.value().insert(std::make_pair(block.key, thumbnail));
            } catch (...) {
                EXCEPTION_NESTED("Failed to generate thumbnail for block: {}", block.key);
            }
        }
    });
}

void AbstractClient::handle(MessageEntityBatch message) {
    WORK([message{std::move(message)}, this]() {
        if (!scene)
            return;

        Log::d("Entity batch received of size: {}", message.entities.size());
        for (const auto& entity : message.entities) {
            scene->insertEntity(entity);
        }
    });
}

void AbstractClient::dispatch(const Network::Packet packet) {
    try {
        dispatcher.dispatch(packet);
    } catch (...) {
        EXCEPTION_NESTED("Failed to dispatch message");
    }
}

void AbstractClient::eventPacket(const Network::StreamPtr& stream, Network::Packet packet) {
    (void)stream;
    try {
        dispatch(std::move(packet));
    } catch (std::exception& e) {
        BACKTRACE(e, "Client failed to process packet");
    }
}

void Client::eventMouseMoved(const Vector2i& pos) {
    mousePos = pos;

    if (view /* && !gui.inputOverlap(pos)*/) {
        view->eventMouseMoved(pos);
    }

    gui.mouseMoveEvent(pos);
}

void Client::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (view && !gui.inputOverlap(pos)) {
        view->eventMousePressed(pos, button);
    }

    gui.mousePressEvent(pos, button);
}

void Client::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (view /* && !gui.inputOverlap(pos)*/) {
        view->eventMouseReleased(pos, button);
    }

    gui.mouseReleaseEvent(pos, button);
}

void Client::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (view && !gui.inputOverlap(mousePos)) {
        view->eventKeyPressed(key, modifiers);
    }

    if (key == Key::LetterB) {
        if (viewBuild) {
            view = viewSpace.get();

            viewMap.reset();
            viewBuild.reset();

        } else {
            viewBuild = std::make_unique<ViewBuild>(config, *this, store, assetManager);
            view = viewBuild.get();
        }
    }

    if (key == Key::LetterM) {
        if (viewMap) {
            view = viewSpace.get();

            viewMap.reset();
            viewBuild.reset();

        } else {
            viewMap = std::make_unique<ViewMap>(config, *this, store, assetManager);
            view = viewMap.get();
        }
    }

    gui.keyPressEvent(key);
}

void Client::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (view /* && !gui.inputOverlap(mousePos)*/) {
        view->eventKeyReleased(key, modifiers);
    }

    gui.keyReleaseEvent(key);
}

void Client::eventMouseScroll(const int xscroll, const int yscroll) {
    if (view && !gui.inputOverlap(mousePos)) {
        view->eventMouseScroll(xscroll, yscroll);
    }
}
