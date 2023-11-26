#include "MsgpackStream.hpp"

using namespace Engine;

MsgpackStream::MsgpackStream() : CompressionStream{}, packer{static_cast<CompressionStream&>(*this)} {
}
