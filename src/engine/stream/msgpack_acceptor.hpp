#pragma once

#include "decompression_acceptor.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API MsgpackAcceptor : public DecompressionAcceptor {
public:
    MsgpackAcceptor();
    NON_COPYABLE(MsgpackAcceptor);
    NON_MOVEABLE(MsgpackAcceptor);

protected:
    void writeDecompressed(const char* data, size_t length) override;

    virtual void receiveObject(msgpack::object_handle oh) = 0;

private:
    msgpack::unpacker unp;
};
} // namespace Engine
