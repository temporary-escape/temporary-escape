#pragma once
#include "NetworkStream.hpp"
#include <memory>
#include <vector>

namespace Scissio::Network {
class SCISSIO_API Session {
public:
    explicit Session(const uint64_t id) : id(id) {
    }

    virtual ~Session() = default;

    virtual void disconnect() {
        for (auto& stream : streams) {
            stream->disconnect();
        }
        streams.clear();
    }

    template <typename T> void send(const size_t channel, const T& message) {
        streams.at(channel)->send(message, id);
    }

    void addStream(StreamPtr stream) {
        streams.push_back(std::move(stream));
    }

    const std::vector<StreamPtr>& getStreams() const {
        return streams;
    }

    uint64_t getSessionId() const {
        return id;
    }

    void setSessionId(const uint64_t id) {
        this->id = id;
    }

private:
    uint64_t id;
    std::vector<StreamPtr> streams;
};

using SessionPtr = std::shared_ptr<Session>;
using SessionWeak = std::weak_ptr<Session>;
} // namespace Scissio::Network
