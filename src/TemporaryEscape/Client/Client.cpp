#include "Client.hpp"
#include "../Assets/AssetManager.hpp"
#include "../Network/NetworkTcpClient.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Server/Messages.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

#define DISPATCH_FUNC(M, T, F) std::bind(static_cast<void (T::*)(M)>(&T::F), this, std::placeholders::_1)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Client, handle));
#define MESSAGE_DISPATCH_FETCH(M)                                                                                      \
    dispatcher.add<M::Response>(std::bind(&Client::handleFetch<M>, this, std::placeholders::_1));

using namespace Engine;

std::atomic<uint64_t> Client::nextRequestId(1);

Client::Client(Config& config, const std::string& address, const int port)
    : listener(std::make_unique<EventListener>(*this)), requestsNextId(1) {

    MESSAGE_DISPATCH(MessageServerHello);
    MESSAGE_DISPATCH(MessageLoginResponse);
    MESSAGE_DISPATCH(MessageStatusResponse);
    MESSAGE_DISPATCH(MessageSectorChanged);
    MESSAGE_DISPATCH(MessageEntitySync);
    MESSAGE_DISPATCH(MessageEntityDeltas);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxy);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxySystems);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxyRegions);
    MESSAGE_DISPATCH_FETCH(MessageFetchCurrentLocation);
    MESSAGE_DISPATCH_FETCH(MessageFetchSystemPlanets);

    const auto profilePath = config.userdataPath / Path("profile.xml");
    if (Fs::exists(profilePath)) {
        Xml::loadAsXml(profilePath, localProfile);
    } else {
        localProfile.secret = randomId();
        localProfile.name = "Some Player";
        Xml::saveAsXml(profilePath, "profile", localProfile);
    }

    network = std::make_shared<Network::TcpClient>(*listener, address, port);

    auto future = connected.future();
    if (future.waitFor(std::chrono::milliseconds(1000)) != std::future_status::ready) {
        EXCEPTION("Timeout while waiting for server connection");
    }
    future.get();

    MessageStatusRequest req{};
    req.timePoint = std::chrono::system_clock::now();
    send(req);
}

Client::~Client() {
}

void Client::eventPacket(Network::Packet packet) {
    try {
        stats.network.packetsReceived++;
        dispatcher.dispatch(packet);
    } catch (...) {
        EXCEPTION_NESTED("Failed to dispatch message");
    }
}

void Client::eventConnect() {
}

void Client::eventDisconnect() {
}

void Client::update() {
    sync.restart();
    sync.run();

    if (scene) {
        const auto now = std::chrono::steady_clock::now();
        auto timeDiff = now - lastTimePoint;
        lastTimePoint = now;
        if (timeDiff > std::chrono::milliseconds(100)) {
            timeDiff = std::chrono::milliseconds(100);
        }
        const auto delta = std::chrono::duration_cast<std::chrono::microseconds>(timeDiff).count() / 1000000.0f;
        scene->update(delta);

        /*Vector3 target;

        for (const auto& entity : scene->getEntities()) {
            try {
                auto component = entity->getComponent<ComponentModel>();
                if (component->getModel()->getName() == "model_frame") {
                    entity->translate(Vector3{0.00f, 0.0f, 0.01f});
                    target = entity->getPosition();
                }
            } catch (std::out_of_range&) {
            }
        }

        for (const auto& entity : scene->getEntities()) {
            try {
                auto component = entity->getComponent<ComponentTurret>();
                component->setTarget(target);
            } catch (std::out_of_range&) {
            }
        }*/
    }
}

void Client::handle(MessageServerHello res) {
    MessageLoginRequest req{};
    req.secret = localProfile.secret;
    req.name = localProfile.name;
    req.password = "password";

    send(req);
}

void Client::handle(MessageLoginResponse res) {
    if (!res.error.empty()) {
        connected.reject<std::runtime_error>(res.error);
    } else {
        playerId = res.playerId;
        connected.resolve();
    }
}

void Client::handle(MessageStatusResponse res) {
    const auto now = std::chrono::system_clock::now();
    const auto diff = now - res.timePoint;
    stats.network.latencyMs.store(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());

    worker1s.post([this]() { network->send(MessageStatusRequest{std::chrono::system_clock::now()}); });
}

void Client::handle(MessageSectorChanged res) {
    sync.post([this, res = std::move(res)]() {
        try {
            Log::i(CMP, "Sector has changed, creating new scene");
            camera.reset();
            scene.reset();
            scene = std::make_unique<Scene>();

            camera = std::make_shared<Entity>();
            auto cmp = camera->addComponent<ComponentCameraTurntable>();
            auto userInput = camera->addComponent<ComponentUserInput>(*cmp);
            cmp->setPrimary(true);
            cmp->setProjection(70.0f);
            scene->addEntity(camera);

            store.player.location = res.location;
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to process sector changed response");
        }
    });
}

void Client::handle(MessageEntitySync res) {
    sync.post([this, res = std::move(res)]() {
        try {
            if (!scene) {
                EXCEPTION("Client scene is not initialized");
            }
            for (auto& entity : res.entities) {
                scene->addEntity(entity);
                if (auto ship = entity->getComponentOpt<ComponentGrid>(); ship.has_value()) {
                    camera->getComponent<ComponentCameraTurntable>()->follow(entity);
                }
            }
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to process entity sync response");
        }
    });
}

void Client::handle(MessageEntityDeltas res) {
    sync.post([this, res = std::move(res)]() {
        try {
            if (!scene) {
                EXCEPTION("Client scene is not initialized");
            }
            for (auto& delta : res.deltas) {
                scene->updateEntity(delta);
            }
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to process entity sync response");
        }
    });
}

template <typename Message, typename T> void Client::handleFetch(MessageFetchResponse<T> res) {
    try {
        if (!res.id) {
            return;
        }

        AbstractRequestPtr abstractReq = nullptr;

        {
            std::lock_guard<std::mutex> lock{requestsMutex};
            const auto it = requests.find(res.id);

            if (it == requests.end()) {
                EXCEPTION("Got response for unknown fetch request id: '{}' type: '{}'", res.id, typeid(T).name());
            } else {
                abstractReq = it->second;
            }

            if (!abstractReq) {
                requests.erase(it);
                return;
            }
        }

        const auto req = std::dynamic_pointer_cast<Request<Message, T>>(abstractReq);
        if (!req) {
            EXCEPTION("Got response for invalid type fetch request id: '{}' type: '{}'", res.id, typeid(T).name());
            return;
        }

        req->append(std::move(res.data));
        if (res.token.empty()) {
            sync.post([this, req]() {
                try {
                    req->complete();
                } catch (std::exception& e) {
                    BACKTRACE(CMP, e, "Failed to process fetch request");
                }
            });
        } else {
            auto& msg = req->getMessage();
            msg.token = res.token;
            send(msg);
        }

    } catch (std::exception& e) {
        BACKTRACE(CMP, e, "Failed to process MessageFetchResponse<T> response");
    }
}

void Client::fetchGalaxy() {
    MessageFetchGalaxy req;
    req.galaxyId = store.player.location.value().galaxyId;

    fetch(req, [this](GalaxyData item) {
        store.galaxy.galaxy = std::move(item);
        store.galaxy.galaxy.markChanged();
    });
}

void Client::fetchGalaxyRegions() {
    MessageFetchGalaxyRegions req;
    req.galaxyId = store.player.location.value().galaxyId;

    fetch(req, [this](std::vector<RegionData> items) {
        store.galaxy.regions.value().clear();
        for (const auto& item : items) {
            store.galaxy.regions.value().insert(std::make_pair(item.id, item));
        }

        store.galaxy.regions.markChanged();
    });
}

void Client::fetchGalaxySystems() {
    MessageFetchGalaxySystems req;
    req.galaxyId = store.player.location.value().galaxyId;

    fetch(req, [this](std::vector<SystemData> items) {
        store.galaxy.systems.value().clear();
        for (const auto& item : items) {
            store.galaxy.systems.value().insert(std::make_pair(item.id, item));
        }

        store.galaxy.systems.markChanged();
    });
}
