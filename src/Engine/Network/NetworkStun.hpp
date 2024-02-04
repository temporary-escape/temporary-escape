#pragma once

#include "NetworkUtils.hpp"
#include <queue>
#include <vector>

namespace Engine {
class ENGINE_API NetworkStunClient {
public:
    struct Result {
        asio::ip::udp::endpoint endpoint;
    };

    using Callback = std::function<void(const Result&)>;

    enum class StunMessageType : uint16_t {
        Request = 0x0001,
        Response = 0x0101,
    };

    NetworkStunClient(const Config& config, asio::io_service& service, asio::io_service::strand& strand,
                      asio::ip::udp::socket& socket);
    ~NetworkStunClient();

    void send(Callback callback);

    bool isValid(const void* data, size_t size) const;
    void parse(const void* data, size_t size);
    bool isRunning() const {
        return !servers.empty() && !requests.empty();
    }
    const asio::ip::udp::endpoint& getResult() const;

private:
    struct Request {
        Callback callback;
        std::string nonce;
    };

    using RequestPtr = std::shared_ptr<Request>;

    // https://datatracker.ietf.org/doc/html/rfc5389#section-6
    struct StunRequest {
        StunMessageType messageType;
        uint16_t messageLength;
        int32_t magicCookie;
        std::array<uint8_t, 12> transactionId;
    };

    static_assert(sizeof(StunMessageType) == 2);
    static_assert(sizeof(StunRequest) == 2 + 2 + 4 + 12);

    struct StunResponse {
        StunMessageType messageType;
        uint16_t messageLength;
        int32_t magicCookie;
        std::array<uint8_t, 12> transactionId;
        std::array<uint8_t, 1000> attributes;
    };

    static_assert(sizeof(StunResponse) == 2 + 2 + 4 + 12 + 1000);

    static StunRequest createStunRequest(const std::string& nonce);

    void doQuery(const RequestPtr& req);
    void doRequest(const RequestPtr& req);
    void cancelRequest(const RequestPtr& req);

    const Config& config;
    asio::io_service& service;
    asio::io_service::strand& strand;
    asio::ip::udp::socket& socket;
    asio::ip::udp::resolver resolver;
    std::queue<std::string> servers;
    std::unique_ptr<asio::ip::udp::resolver::query> query;
    std::vector<asio::ip::udp::endpoint> endpoints;
    asio::ip::udp::endpoint endpoint;
    bool chosen{false};
    StunRequest request{};
    std::optional<asio::ip::udp::endpoint> mapped;
    std::unique_ptr<asio::steady_timer> deadline;
    std::unordered_map<std::string, RequestPtr> requests;
};
} // namespace Engine
