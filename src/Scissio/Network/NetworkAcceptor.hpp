#pragma once
#include "NetworkStream.hpp"
#include <memory>

namespace Scissio::Network {
class Server;

class SCISSIO_API Acceptor {
public:
    Acceptor() = default;
    virtual ~Acceptor() = default;

    virtual void start() = 0;

    virtual void receive(const StreamPtr& stream, Packet packet) = 0;
};

using AcceptorPtr = std::shared_ptr<Acceptor>;
} // namespace Scissio::Network
