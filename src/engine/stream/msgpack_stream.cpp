#include "msgpack_stream.hpp"

using namespace Engine;

MsgpackStream::MsgpackStream(const size_t blockBytes) :
    CompressionStream{blockBytes}, packer{static_cast<CompressionStream&>(*this)} {
}
