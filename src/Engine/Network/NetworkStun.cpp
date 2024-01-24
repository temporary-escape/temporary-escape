#include "NetworkStun.hpp"
#include "../Utils/StringUtils.hpp"
#include <random>

using namespace Engine;

template <typename T> static void reverseEndianness(T& value) {
    std::reverse(reinterpret_cast<char*>(&value), reinterpret_cast<char*>(&value) + sizeof(T));
}

template <typename T> static T toBigEndian(T value) {
    std::reverse(reinterpret_cast<char*>(&value), reinterpret_cast<char*>(&value) + sizeof(T));
    return value;
}

static constexpr int32_t magicCookie = 0x2112A442;
static constexpr int16_t attrTypeMappedAddress = 0x0020;
static const int16_t stunResponseTypeRev = toBigEndian(static_cast<int16_t>(StunMessageType::Response));
static const int32_t magicCookieRev = toBigEndian(magicCookie);

static auto logger = createLogger(LOG_FILENAME);

NetworkStunRequest Engine::createStunRequest() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<uint8_t> dist;

    NetworkStunRequest req{};
    req.messageType = StunMessageType::Request;
    req.messageLength = 0;
    req.magicCookie = magicCookie;

    reverseEndianness(req.messageType);
    reverseEndianness(req.messageLength);
    reverseEndianness(req.magicCookie);

    for (auto& b : req.transactionId) {
        b = dist(rng);
    }

    return req;
}

NetworkStunClient::NetworkStunClient(const Config& config, asio::io_service& service, asio::io_service::strand& strand,
                                     asio::ip::udp::socket& socket) :
    config{config}, service{service}, strand{strand}, socket{socket}, resolver{service} {
}

NetworkStunClient::~NetworkStunClient() = default;

void NetworkStunClient::sendRequest() {
    strand.post([this]() {
        for (const auto& address : config.network.stunServers) {
            servers.push(address);
        }

        doQuery();
    });
}

void NetworkStunClient::doQuery() {
    if (servers.empty()) {
        return;
    }

    logger.debug("STUN client resolving host: {}", servers.front());

    const auto tokens = splitLast(servers.front(), ":");
    if (tokens.size() != 2) {
        logger.error("Invalid STUN address: {}", servers.front());
        return;
    }

    servers.pop();

    query = std::make_unique<asio::ip::udp::resolver::query>(asio::ip::udp::v4(), tokens[0], tokens[1]);

    using Result = asio::ip::udp::resolver::results_type;
    resolver.async_resolve(*query, strand.wrap([this](const asio::error_code ec, Result result) {
        if (ec) {
            logger.error("STUN client failed to resolve query error: {}", ec.message());
        } else {
            endpoints.clear();
            for (const auto& e : result) {
                if (e.endpoint().address().is_v4()) {
                    logger.debug("STUN client resolved host: {}", e.endpoint());
                    endpoints.push_back(e.endpoint());
                }
            }

            doRequest();
        }
    }));
}

void NetworkStunClient::doRequest() {
    if (endpoints.empty()) {
        doQuery();
        return;
    }

    endpoint = endpoints.back();
    endpoints.pop_back();

    request = createStunRequest();
    auto buff = asio::buffer(&request, sizeof(NetworkStunRequest));

    if (deadline) {
        asio::error_code ec;
        deadline->cancel(ec);
        if (ec) {
            logger.error("STUN client cancel deadline error: {}", ec.message());
        }
    }

    deadline = std::make_unique<asio::steady_timer>(service, deadlineInterval);
    deadline->async_wait(strand.wrap([this](const asio::error_code ec) {
        if (ec) {
            logger.error("STUN client deadline timer error: {}", ec.message());
        } else {
            if (!mapped) {
                logger.warn("STUN client timeout waiting for response");
                doRequest();
            }
        }
    }));

    logger.info("STUN client sending request to: {}", endpoint);
    socket.async_send_to(buff, endpoint, strand.wrap([this](const asio::error_code ec, const size_t sent) {
        if (ec) {
            logger.error("STUN client failed send request error: {}", ec.message());
        }
    }));
}

bool NetworkStunClient::isValid(const void* data, size_t size) const {
    if (size < 32) {
        return false;
    }

    const auto src = reinterpret_cast<const char*>(data);

    // Is the message starting with a STUN response type?
    if (std::memcmp(src, &stunResponseTypeRev, sizeof(stunResponseTypeRev)) != 0) {
        return false;
    }

    // Does the message contain a valid magic cookie?
    if (std::memcmp(src + 4, &magicCookieRev, sizeof(magicCookieRev)) != 0) {
        return false;
    }

    return true;
}

// Adapted from: https://gist.github.com/jyaif/e0db3a680443730c05ca36be26f22c93
void NetworkStunClient::parse(const void* data, size_t size) {
    if (size < 32 || size > sizeof(NetworkStunResponse)) {
        return;
    }

    NetworkStunResponse res{};
    std::memcpy(&res, data, size);

    reverseEndianness(res.messageType);
    reverseEndianness(res.messageLength);
    reverseEndianness(res.magicCookie);

    // Is the chosen transaction ID valid?
    if (std::memcmp(res.transactionId.data(), request.transactionId.data(), request.transactionId.size()) != 0) {
        logger.warn("STUN client received wrong transaction ID");
        return;
    }

    if (res.messageLength < 4) {
        logger.warn("STUN client response is too short");
    }

    size_t i{0};
    while (i < res.messageLength) {
        auto& type = *reinterpret_cast<uint16_t*>(&res.attributes[i]);
        auto& length = *reinterpret_cast<uint16_t*>(&res.attributes[i + 2]);

        reverseEndianness(type);
        reverseEndianness(length);

        if (i + length > res.messageLength) {
            break;
        }

        if (type == attrTypeMappedAddress) {
            auto& port = *reinterpret_cast<uint16_t*>(&res.attributes[i + 6]);
            reverseEndianness(port);

            port ^= (magicCookie >> 16);

            const auto ip = std::to_string(res.attributes[i + 8] ^ ((magicCookie & 0xff000000) >> 24)) + "." +
                            std::to_string(res.attributes[i + 9] ^ ((magicCookie & 0x00ff0000) >> 16)) + "." +
                            std::to_string(res.attributes[i + 10] ^ ((magicCookie & 0x0000ff00) >> 8)) + "." +
                            std::to_string(res.attributes[i + 11] ^ ((magicCookie & 0x000000ff) >> 0));

            logger.info("STUN client received address: {}:{}", ip, port);

            mapped = asio::ip::udp::endpoint{asio::ip::address::from_string(ip), port};
        }

        i += 4 + length;
    }
}

const asio::ip::udp::endpoint& NetworkStunClient::getResult() const {
    if (!mapped) {
        EXCEPTION("STUN client has no result available");
    }
    return *mapped;
}
