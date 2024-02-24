#pragma once

#include "../Network/NetworkStream.hpp"

namespace Engine {
class ENGINE_API Session {
public:
    enum Flags : uint64_t {
        PingSent,
    };

    explicit Session(std::string playerId, const NetworkStreamPtr& stream) :
        playerId{std::move(playerId)}, stream{stream}, lastPingTime{}, flags{0} {
    }

    [[nodiscard]] const std::string& getPlayerId() const {
        return playerId;
    }

    template <typename T> void send(T& msg) {
        if (auto ptr = stream.lock(); ptr != nullptr) {
            ptr->send(msg, 0);
        } else {
            connected = false;
        }
    }

    [[nodiscard]] const std::chrono::steady_clock::time_point& getLastPingTime() const {
        return lastPingTime;
    }

    void setLastPingTime(const std::chrono::steady_clock::time_point& value) {
        lastPingTime = value;
    }

    void setFlag(Flags flag) {
        flags |= flag;
    }

    void clearFlag(Flags flag) {
        flags &= ~flag;
    }

    [[nodiscard]] bool hasFlag(Flags flag) const {
        return flags & flag;
    }

    void close() {
        if (auto ptr = stream.lock(); ptr != nullptr) {
            ptr->close();
        }
        connected = false;
    }

    [[nodiscard]] std::shared_ptr<NetworkStream> getStream() const {
        return stream.lock();
    }

private:
    std::string playerId;
    std::weak_ptr<NetworkStream> stream;
    std::chrono::steady_clock::time_point lastPingTime;
    uint64_t flags;
    bool connected{true};
};

using SessionPtr = std::shared_ptr<Session>;
} // namespace Engine
