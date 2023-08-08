#include "msgpack_acceptor.hpp"

using namespace Engine;

MsgpackAcceptor::MsgpackAcceptor() : DecompressionAcceptor{} {
}

void MsgpackAcceptor::writeDecompressed(const char* data, size_t length) {
    unp.reserve_buffer(length);
    std::memcpy(unp.buffer(), data, length);
    unp.buffer_consumed(length);

    msgpack::object_handle oh{};
    while (unp.next(oh)) {
        receiveObject(std::move(oh));
        oh = msgpack::object_handle{};
    }
}
