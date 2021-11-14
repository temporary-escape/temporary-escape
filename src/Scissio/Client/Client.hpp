#pragma once

#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Server/Messages.hpp"
#include "../Server/Schemas.hpp"
#include "../Utils/Worker.hpp"
#include "Request.hpp"

namespace Scissio {
class SCISSIO_API Client {
public:
    explicit Client(Config& config, const std::string& address, int port);
    virtual ~Client();

    Future<void> login(const std::string& password);
    RequestPtr<SystemData> fetchSystems(const std::string& galaxyId) {
        return fetch<SystemData>(galaxyId);
    }

    template <typename T> RequestPtr<T> fetch(const std::string& prefix) {
        auto request = std::make_shared<Request<T>>(nextRequestId.fetch_add(1));

        {
            std::lock_guard<std::mutex> lock{requestsMutex};
            requests.insert(std::make_pair(request->getId(), request));
        }

        MessageFetchRequest<T> req;
        req.id = request->getId();
        req.prefix = prefix;
        network->send(req);

        return request;
    }

    void eventPacket(Network::Packet packet);
    void eventConnect();
    void eventDisconnect();

    void update();

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

    void handle(MessageLoginResponse res);

    template <typename T> void handle(MessageFetchResponse<T> res);

    static std::atomic<uint64_t> nextRequestId;

    std::unique_ptr<EventListener> listener;
    std::shared_ptr<Network::Client> network;
    Network::MessageDispatcher<> dispatcher;
    uint64_t secret;
    std::string playerId;

    Promise<void> connectPromise;
    Promise<void> loginPromise;

    asio::io_service sync;
    // BackgroundWorker background;

    std::mutex requestsMutex;
    std::unordered_map<uint64_t, AbstractRequestPtr> requests;
};
} // namespace Scissio
