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
static const int16_t stunResponseTypeRev =
    toBigEndian(static_cast<int16_t>(NetworkStunClient::StunMessageType::Response));
static const int32_t magicCookieRev = toBigEndian(magicCookie);
static constexpr asio::chrono::milliseconds deadlineInterval{1000};

static auto logger = createLogger(LOG_FILENAME);

static std::string createNonce() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> dist{'a', 'z'};

    std::string res;
    res.resize(12);
    for (auto& c : res) {
        c = static_cast<char>(dist(rng));
    }

    return res;
}

NetworkStunClient::StunRequest NetworkStunClient::createStunRequest(const std::string& nonce) {
    StunRequest req{};
    req.messageType = StunMessageType::Request;
    req.messageLength = 0;
    req.magicCookie = magicCookie;

    reverseEndianness(req.messageType);
    reverseEndianness(req.messageLength);
    reverseEndianness(req.magicCookie);

    for (size_t i = 0; i < 12; i++) {
        req.transactionId[i] = nonce[i];
    }

    return req;
}

NetworkStunClient::NetworkStunClient(const Config& config, asio::io_service& service, asio::io_service::strand& strand,
                                     asio::ip::udp::socket& socket) :
    config{config}, service{service}, strand{strand}, socket{socket}, resolver{service} {
}

NetworkStunClient::~NetworkStunClient() = default;

void NetworkStunClient::send(Callback callback) {
    strand.post([this, c = std::move(callback)]() {
        logger.info("STUN client sending request");

        const auto nonce = createNonce();
        auto req = std::make_shared<Request>(Request{std::move(c), nonce});
        requests.emplace(nonce, req);

        while (!servers.empty()) {
            servers.pop();
        }

        for (const auto& address : config.network.stunServers) {
            servers.push(address);
        }

        doQuery(req);
    });
}

void NetworkStunClient::cancelRequest(const NetworkStunClient::RequestPtr& req) {
    const auto it = requests.find(req->nonce);
    if (it != requests.end()) {
        requests.erase(it);
    }
}

void NetworkStunClient::doQuery(const RequestPtr& req) {
    if (servers.empty()) {
        cancelRequest(req);
        return;
    }

    logger.debug("STUN client resolving host: {}", servers.front());

    const auto tokens = splitLast(servers.front(), ":");
    if (tokens.size() != 2) {
        logger.error("Invalid STUN address: {}", servers.front());
        cancelRequest(req);
        return;
    }

    servers.pop();

    query = std::make_unique<asio::ip::udp::resolver::query>(asio::ip::udp::v4(), tokens[0], tokens[1]);

    using Result = asio::ip::udp::resolver::results_type;
    resolver.async_resolve(*query, strand.wrap([this, req](const asio::error_code ec, const Result& result) {
        if (ec) {
            logger.error("STUN client failed to resolve query error: {}", ec.message());
            cancelRequest(req);
        } else {
            endpoints.clear();
            for (const auto& e : result) {
                if (e.endpoint().address().is_v4()) {
                    if (socket.local_endpoint().address().is_v6()) {
                        endpoints.push_back(toIPv6(e));
                    } else {
                        endpoints.push_back(e.endpoint());
                    }
                    logger.debug("STUN client resolved host: {}", endpoints.back());
                }
            }

            doRequest(req);
        }
    }));
}

void NetworkStunClient::doRequest(const RequestPtr& req) {
    if (endpoints.empty()) {
        doQuery(req);
        return;
    }

    endpoint = endpoints.back();
    endpoints.pop_back();

    request = createStunRequest(req->nonce);
    auto buff = asio::buffer(&request, sizeof(StunRequest));

    if (deadline) {
        asio::error_code ec;
        deadline->cancel(ec);
        if (ec) {
            logger.error("STUN client cancel deadline error: {}", ec.message());
        }
    }

    deadline = std::make_unique<asio::steady_timer>(service, deadlineInterval);
    deadline->async_wait(strand.wrap([this, req](const asio::error_code ec) {
        if (ec) {
            logger.error("STUN client deadline timer error: {}", ec.message());
            cancelRequest(req);
        } else {
            if (!mapped) {
                logger.warn("STUN client timeout waiting for response");
                doRequest(req);
            }
        }
    }));

    logger.debug("STUN client sending request to: {}", endpoint);
    socket.async_send_to(buff, endpoint, strand.wrap([](const asio::error_code ec, const size_t sent) {
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
    if (size < 32 || size > sizeof(StunResponse)) {
        return;
    }

    StunResponse res{};
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
            break;
        }

        i += 4 + length;
    }

    if (mapped) {
        const auto nonce = std::string{reinterpret_cast<const char*>(res.transactionId.data()), 12};

        const auto it = requests.find(nonce);
        if (it != requests.end()) {
            it->second->callback(Result{*mapped});
            requests.erase(it);
        }
    }
}

const asio::ip::udp::endpoint& NetworkStunClient::getResult() const {
    if (!mapped) {
        EXCEPTION("STUN client has no result available");
    }
    return *mapped;
}
