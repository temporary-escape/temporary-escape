#pragma once

#include "Lz4FileWriter.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API MsgpackFileWriter : public Lz4FileWriter, public msgpack::packer<Lz4FileWriter> {
public:
    explicit MsgpackFileWriter(const Path& path);
};
} // namespace Engine
