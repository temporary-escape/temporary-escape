#pragma once

#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Server/Messages.hpp"
#include "../Server/Schemas.hpp"
#include "../Utils/Worker.hpp"

namespace Scissio {
class SCISSIO_API Client {
public:
    static std::atomic<uint64_t> nextRequestId;

    class AbstractRequest {
    public:
        virtual ~AbstractRequest() = default;
        virtual void complete() = 0;
    };

    using AbstractRequestPtr = std::shared_ptr<AbstractRequest>;

    template <typename T> class Request : public AbstractRequest {
    public:
        using type = T;

        Request() : id(Client::nextRequestId.fetch_add(1)), callback(nullptr) {
        }

        ~Request() override = default;

        const std::vector<T>& value() const {
            return data;
        }

        void then(std::function<void(const std::vector<T>&)> fn) {
            callback = std::move(fn);
        }

        uint64_t getId() {
            return id;
        }

        void append(const std::vector<T>& data) {
            this->data.insert(this->data.end(), data.begin(), data.end());
        }

        void complete() override {
            if (callback) {
                callback(data);
            }
        }

    private:
        uint64_t id;
        std::vector<T> data;
        std::function<void(const std::vector<T>&)> callback;
    };

    template <typename T> using RequestPtr = std::shared_ptr<Request<T>>;

    explicit Client(Config& config, const std::string& address, int port);
    virtual ~Client();

    Future<void> login(const std::string& password);
    RequestPtr<SystemData> fetchSystems() {
        return fetch<SystemData>();
    }

    template <typename T> RequestPtr<T> fetch() {
        auto request = std::make_shared<Request<T>>();

        {
            std::lock_guard<std::mutex> lock{requestsMutex};
            requests.insert(std::make_pair(request->getId(), request));
        }

        MessageFetchRequest<T> req;
        req.id = request->getId();
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
