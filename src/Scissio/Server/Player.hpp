#pragma once

#include "../Network/NetworkStream.hpp"

namespace Scissio {
class Player {
public:
    explicit Player(Network::StreamPtr stream) : stream(stream) {
    }

    const std::string& getId() const {
        return id;
    }

    void setId(std::string value) {
        id = std::move(value);
    }

    const std::string& getCurrentLocationId() const {
        return location;
    }

    void setLocation(std::string value) {
        location = std::move(value);
    }

    template <typename T> void send(const T& msg) {
        stream->send(msg);
    }

private:
    Network::StreamPtr stream;
    std::string id;
    std::string location;
};

using PlayerPtr = std::shared_ptr<Player>;
} // namespace Scissio
