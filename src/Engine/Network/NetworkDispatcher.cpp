#include "NetworkDispatcher.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void NetworkDispatcher2::onObjectReceived(NetworkStreamPtr peer, ObjectHandlePtr oh) {
    uint64_t id;
    oh->get().via.array.ptr[0].convert(id);

    auto it = handlers.find(id);
    if (it == handlers.end()) {
        logger.error("No such handler for message id: {}", id);
        return;
    }

    try {
        it->second.fn(std::move(peer), std::move(oh));
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to handle message: {}", it->second.name);
    }
}
