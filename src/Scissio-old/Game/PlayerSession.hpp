#pragma once

#include <utility>

#include "../Network/NetworkStream.hpp"

namespace Scissio {
class PlayerSession : public std::enable_shared_from_this<PlayerSession> {
public:
    explicit PlayerSession(std::shared_ptr<Network::Stream> stream, const int64_t uid)
        : stream(std::move(stream)), uid(uid) {
    }
    virtual ~PlayerSession() = default;

    template <typename T> void send(const T& message) {
        stream->send(message);
    }

    int64_t getPlayerId() const {
        return uid;
    }

    const std::shared_ptr<Network::Stream>& getStream() const {
        return stream;
    }

private:
    std::shared_ptr<Network::Stream> stream;
    int64_t uid;
};

using PlayerSessionPtr = std::shared_ptr<PlayerSession>;
} // namespace Scissio
