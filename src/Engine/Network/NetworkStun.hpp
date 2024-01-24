#pragma once

#include "NetworkUtils.hpp"
#include <queue>
#include <vector>

namespace Engine {
enum class StunMessageType : uint16_t {
    Request = 0x0001,
    Response = 0x0101,
};

// https://datatracker.ietf.org/doc/html/rfc5389#section-6
struct NetworkStunRequest {
    StunMessageType messageType;
    uint16_t messageLength;
    int32_t magicCookie;
    std::array<uint8_t, 12> transactionId;
};

static_assert(sizeof(StunMessageType) == 2);
static_assert(sizeof(NetworkStunRequest) == 2 + 2 + 4 + 12);

struct NetworkStunResponse {
    StunMessageType messageType;
    uint16_t messageLength;
    int32_t magicCookie;
    std::array<uint8_t, 12> transactionId;
    std::array<uint8_t, 1000> attributes;
};

static_assert(sizeof(NetworkStunResponse) == 2 + 2 + 4 + 12 + 1000);

NetworkStunRequest createStunRequest();

class ENGINE_API NetworkStunClient {
public:
    NetworkStunClient(const Config& config, asio::io_service& service, asio::io_service::strand& strand,
                      asio::ip::udp::socket& socket);
    ~NetworkStunClient();

    void sendRequest();
    bool isValid(const void* data, size_t size) const;
    void parse(const void* data, size_t size);
    bool isRunning() const {
        return !servers.empty() && !mapped;
    }
    bool hasResult() const {
        return mapped.has_value();
    }
    const asio::ip::udp::endpoint& getResult() const;

private:
    static constexpr asio::chrono::milliseconds deadlineInterval{500};

    void doQuery();
    void doRequest();

    const Config& config;
    asio::io_service& service;
    asio::io_service::strand& strand;
    asio::ip::udp::socket& socket;
    asio::ip::udp::resolver resolver;
    std::queue<std::string> servers;
    std::unique_ptr<asio::ip::udp::resolver::query> query;
    std::vector<asio::ip::udp::endpoint> endpoints;
    asio::ip::udp::endpoint endpoint;
    NetworkStunRequest request{};
    std::optional<asio::ip::udp::endpoint> mapped;
    std::unique_ptr<asio::steady_timer> deadline;
};
} // namespace Engine
