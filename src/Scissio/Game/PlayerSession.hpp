#pragma once
#include "../Network/NetworkSession.hpp"

namespace Scissio {
class PlayerSession : public Network::Session, std::enable_shared_from_this<PlayerSession> {
public:
    explicit PlayerSession(const uint64_t sessionId, const int64_t uid) : Network::Session(sessionId), uid(uid) {
    }
    virtual ~PlayerSession() = default;

    int64_t getPlayerId() const {
        return uid;
    }

private:
    int64_t uid;
};

using PlayerSessionPtr = std::shared_ptr<PlayerSession>;
} // namespace Scissio
