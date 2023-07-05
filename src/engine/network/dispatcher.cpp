#include "dispatcher.hpp"
#include "peer.hpp"
#include <iostream>

using namespace Engine::Network;

Dispatcher::Dispatcher(ErrorHandler& errorHandler) : errorHandler{errorHandler} {
}

void Dispatcher::dispatch(const PeerPtr& peer, const uint64_t id, const uint64_t reqId, ObjectHandlePtr oh) {
    const auto it = handlers.find(id);
    if (it != handlers.end()) {
        it->second(peer, reqId, std::move(oh));
    } else {
        errorHandler.onError(peer, ::make_error_code(Error::UnexpectedRequest));
    }
}
