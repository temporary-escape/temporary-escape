#pragma once

#include "../Config.hpp"
#include "../Library.hpp"
#include "../Network/NetworkTcpClient.hpp"

namespace Scissio {
class SCISSIO_API Client : public Network::TcpClient {
public:
    explicit Client(Config& config, const std::string& address, int port);
    ~Client() override;

    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override;

private:
    uint64_t uid;
};
} // namespace Scissio
