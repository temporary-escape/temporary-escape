#pragma once
#include "NetworkEventListener.hpp"
#include "NetworkStream.hpp"
#include <memory>

namespace Scissio::Network {
class Server;

class SCISSIO_API Acceptor {
public:
    explicit Acceptor(EventListener& listener);
    virtual ~Acceptor() = default;

    virtual void start() = 0;

    void eventPacket(const StreamPtr& stream, Packet packet);
    void eventDisconnect(const StreamPtr& stream);

protected:
    EventListener& listener;
};

using AcceptorPtr = std::shared_ptr<Acceptor>;
} // namespace Scissio::Network
