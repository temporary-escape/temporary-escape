#pragma once

#include "lz4_file_writer.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API MsgpackFileWriter : public Lz4FileWriter, public msgpack::packer<Lz4FileWriter> {
public:
    explicit MsgpackFileWriter(const Path& path);
};
} // namespace Engine
