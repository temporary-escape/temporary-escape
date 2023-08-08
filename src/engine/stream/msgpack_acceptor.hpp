#pragma once

#include "decompression_acceptor.hpp"
#include <msgpack.hpp>

namespace Engine {
class MsgpackAcceptor : public DecompressionAcceptor {
public:
    explicit MsgpackAcceptor(size_t blockBytes = 1024 * 8);
    NON_COPYABLE(MsgpackAcceptor);
    NON_MOVEABLE(MsgpackAcceptor);

protected:
    void writeDecompressed(const void* data, size_t length) override;

    virtual void receiveObject(std::shared_ptr<msgpack::object_handle> oh) = 0;

private:
    msgpack::unpacker unp;
};
} // namespace Engine
