#pragma once

#include "../library.hpp"
#include <functional>
#include <memory>
#include <system_error>

namespace Engine::Network {
class ENGINE_API Peer;

enum Error {
    HandshakeError,
    UnexpectedResponse,
    BadMessageFormat,
    UnexpectedRequest,
    UnpackError,
    DecompressError,
};

class ENGINE_API ErrorCategory : public std::error_category {
public:
    virtual const char* name() const noexcept;
    virtual std::string message(int ev) const;
    virtual std::error_condition default_error_condition(int ev) const noexcept;
};

static inline const ErrorCategory errorCategory{};

class ENGINE_API ErrorHandler {
public:
    virtual ~ErrorHandler() = default;

    virtual void onError(std::error_code ec);
    virtual void onError(const std::shared_ptr<Peer>& peer, std::error_code ec);
    virtual void onUnhandledException(const std::shared_ptr<Peer>& peer, std::exception_ptr& eptr);

    void setErrorCallback(std::function<void(std::error_code)> fn) {
        onErrorCallback = std::move(fn);
    }

    void setPeerErrorCallback(std::function<void(const std::shared_ptr<Peer>&, std::error_code)> fn) {
        onPeerErrorCallback = std::move(fn);
    }

    void setPeerExceptionCallback(std::function<void(const std::shared_ptr<Peer>&, std::exception_ptr&)> fn) {
        onPeerExceptionCallback = std::move(fn);
    }

private:
    std::function<void(std::error_code)> onErrorCallback;
    std::function<void(const std::shared_ptr<Peer>&, std::error_code)> onPeerErrorCallback;
    std::function<void(const std::shared_ptr<Peer>&, std::exception_ptr&)> onPeerExceptionCallback;
};

inline std::error_code make_error_code(Engine::Network::Error e) {
    return {static_cast<int>(e), Engine::Network::errorCategory};
}
} // namespace Engine::Network

namespace std {
template <> struct is_error_code_enum<Engine::Network::Error> : true_type {};
} // namespace std
