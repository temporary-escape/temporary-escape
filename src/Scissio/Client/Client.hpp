#pragma once

#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Messages.hpp"
#include "../Server/Schemas.hpp"
#include "../Server/Sector.hpp"
#include "../Utils/Worker.hpp"
#include "Request.hpp"
#include "Stats.hpp"
#include "Store.hpp"

namespace Scissio {
class SCISSIO_API Client {
public:
    explicit Client(Config& config, const std::string& address, int port);
    virtual ~Client();

    Future<void> login(const std::string& password);

    template <typename RequestType, typename Callback = typename Request<RequestType>::Callback>
    void fetch(RequestType msg, Callback fn) {
        const auto id = requestsNextId.fetch_add(1);
        msg.id = id;
        const auto req = std::make_shared<Request<RequestType>>(std::move(msg), std::move(fn));

        {
            std::lock_guard<std::mutex> lock{requestsMutex};
            requests.insert(std::make_pair(id, req));
        }

        send(req->getMessage());
    }

    void eventPacket(Network::Packet packet);
    void eventConnect();
    void eventDisconnect();
    void fetchGalaxy();
    void fetchGalaxySystems();
    void fetchGalaxyRegions();

    void update();

    const Stats& getStats() const {
        return stats;
    }

    Scene* getScene() const {
        return scene.get();
    }

    Stats& getStats() {
        return stats;
    }

    Store& getStore() {
        return store;
    }

    template <typename T> void send(const T& message) {
        network->send(message);
        ++stats.network.packetsSent;
    }

private:
    class EventListener : public Network::EventListener {
    public:
        explicit EventListener(Client& client) : client(client) {
        }

        void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override {
            (void)stream;
            client.eventPacket(std::move(packet));
        }

        void eventConnect(const Network::StreamPtr& stream) override {
            (void)stream;
            client.eventConnect();
        }

        void eventDisconnect(const Network::StreamPtr& stream) override {
            (void)stream;
            client.eventDisconnect();
        }

    private:
        Client& client;
    };

    void handle(MessageServerHello res);
    void handle(MessageLoginResponse res);
    void handle(MessageStatusResponse res);
    void handle(MessageSectorChanged res);
    void handle(MessageEntitySync res);
    template <typename Message, typename T = typename Message::Response::ItemType>
    void handleFetch(MessageFetchResponse<T> res);

    static std::atomic<uint64_t> nextRequestId;

    Store store;

    std::unique_ptr<EventListener> listener;
    std::shared_ptr<Network::Client> network;
    Network::MessageDispatcher<> dispatcher;
    uint64_t secret;
    std::string playerId;

    Promise<void> connected;

    asio::io_service sync;
    PeriodicWorker worker1s{std::chrono::milliseconds(1000)};
    // BackgroundWorker background;

    std::atomic<uint64_t> requestsNextId;
    std::mutex requestsMutex;
    std::unordered_map<uint64_t, AbstractRequestPtr> requests;

    Stats stats;

    std::unique_ptr<Scene> scene;
};
} // namespace Scissio
