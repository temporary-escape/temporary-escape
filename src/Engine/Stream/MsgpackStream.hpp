#pragma once

#include "CompressionStream.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API MsgpackStream : public CompressionStream, public msgpack::packer<CompressionStream> {
public:
    MsgpackStream();
    NON_COPYABLE(MsgpackStream);
    NON_MOVEABLE(MsgpackStream);
};
} // namespace Engine
