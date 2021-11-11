#pragma once

#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Server/Messages.hpp"

namespace Scissio {
class SCISSIO_API Client {
public:
    explicit Client(Config& config, const std::string& address, int port);
    virtual ~Client();

    void login(const std::string& password);

    void eventPacket(Network::Packet packet);
    void eventConnect();
    void eventDisconnect();

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

    void handle(MessageLoginResponse req);

    std::unique_ptr<EventListener> listener;
    std::shared_ptr<Network::Client> network;
    Network::MessageDispatcher<> dispatcher;
    uint64_t secret;
    uint64_t playerId;

    Promise<void> connectPromise;
    Promise<void> loginPromise;
};
} // namespace Scissio
