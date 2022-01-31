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

using namespace Scissio;

std::atomic<uint64_t> Client::nextRequestId(1);

uint64_t generatePlayerUid(const Config& config) {
    const auto path = config.userdataPath / Path("uid");
    if (Fs::exists(path)) {
        std::ifstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for reading", path.string());
        }

        const std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        return std::stoull(str);
    } else {
        const auto uid = randomId();

        std::ofstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for writing", path.string());
        }

        f << uid;

        return uid;
    }
}

Client::Client(Config& config, const std::string& address, const int port)
    : listener(std::make_unique<EventListener>(*this)),
      network(std::make_shared<Network::TcpClient>(*listener, address, port)), secret(generatePlayerUid(config)),
      requestsNextId(1) {

    MESSAGE_DISPATCH(MessageServerHello);
    MESSAGE_DISPATCH(MessageLoginResponse);
    MESSAGE_DISPATCH(MessageStatusResponse);
    MESSAGE_DISPATCH(MessageSectorChanged);
    MESSAGE_DISPATCH(MessageEntitySync);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxy);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxySystems);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxyRegions);
    MESSAGE_DISPATCH_FETCH(MessageFetchCurrentLocation);
    MESSAGE_DISPATCH_FETCH(MessageFetchSystemPlanets);

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
        scene->update();
    }
}

void Client::handle(MessageServerHello res) {
    MessageLoginRequest req{};
    req.secret = secret;
    req.name = "Some Player";
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
            scene = std::make_unique<Scene>();

            auto entity = std::make_shared<Entity>();
            auto camera = entity->addComponent<ComponentCameraTurntable>();
            auto userInput = entity->addComponent<ComponentUserInput>(*camera);
            camera->setPrimary(true);
            camera->setProjection(70.0f);
            scene->addEntity(entity);

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
