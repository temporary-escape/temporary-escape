#pragma once

#include "../stream/msgpack_acceptor.hpp"
#include "../stream/msgpack_stream.hpp"
#include "network_dispatcher.hpp"
#include <asio.hpp>
#include <mutex>

namespace Engine {
class ENGINE_API NetworkPeer : public MsgpackAcceptor, public MsgpackStream {
public:
    class LockGuard {
    public:
        explicit LockGuard(NetworkPeer& peer);
        explicit LockGuard(std::mutex& mutex) : lock{mutex} {
        }

    private:
        std::lock_guard<std::mutex> lock;
    };

    virtual ~NetworkPeer() = default;

    virtual bool isConnected() const = 0;
    virtual const std::string& getAddress() const = 0;
    virtual void close() = 0;

    template <typename T> void send(const T& msg, const uint64_t xid) {
        std::lock_guard<std::mutex> lock{sendMutex};
        Detail::packMessage(*this, msg, xid);
    }

    template <typename T> void prepareSend(const uint64_t xid) {
        Detail::prepareMessage<T>(*this, xid);
    }

    std::mutex& getMutex() {
        return sendMutex;
    }

private:
    std::mutex sendMutex;
};

template <typename T> inline void BaseRequest::respond(const T& msg) const {
    peer->send(msg, xid);
}

inline void BaseRequest::respondError(const std::string& msg) const {
    peer->send(msg, xid);
}

inline bool isAsioEofError(const asio::error_code& ec) {
    return ec == asio::error::eof || ec == asio::error::operation_aborted || ec == asio::error::connection_reset;
}

inline NetworkPeer::LockGuard::LockGuard(NetworkPeer& peer) : lock{peer.getMutex()} {
}
} // namespace Engine
