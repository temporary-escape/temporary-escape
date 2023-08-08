#include "msgpack_acceptor.hpp"

using namespace Engine;

MsgpackAcceptor::MsgpackAcceptor(const size_t blockBytes) : DecompressionAcceptor{blockBytes} {
}

void MsgpackAcceptor::writeDecompressed(const void* data, size_t length) {
    unp.reserve_buffer(length);
    std::memcpy(unp.buffer(), data, length);
    unp.buffer_consumed(length);

    auto oh = std::make_shared<msgpack::object_handle>();
    while (unp.next(*oh)) {
        receiveObject(std::move(oh));
        oh = std::make_shared<msgpack::object_handle>();
    }
}
