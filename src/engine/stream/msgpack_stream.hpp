#pragma once

#include "compression_stream.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API MsgpackStream : public CompressionStream, public msgpack::packer<CompressionStream> {
public:
    explicit MsgpackStream(size_t blockBytes = 1024 * 8);
    NON_COPYABLE(MsgpackStream);
    NON_MOVEABLE(MsgpackStream);
};
} // namespace Engine
