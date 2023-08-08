#include "network_dispatcher.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void NetworkDispatcher::onObjectReceived(NetworkPeerPtr peer, ObjectHandlePtr oh) {
    uint64_t id;
    oh->get().via.array.ptr[0].convert(id);

    auto it = handlers.find(id);
    if (it == handlers.end()) {
        EXCEPTION("No such handler for message id: {}", id);
    }

    try {
        it->second.fn(std::move(peer), std::move(oh));
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to handle message: {}", it->second.name);
    }
}