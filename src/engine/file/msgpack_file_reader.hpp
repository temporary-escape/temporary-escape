#pragma once

#include "lz4_file_reader.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API MsgpackFileReader : public Lz4FileReader {
public:
    explicit MsgpackFileReader(const Path& path);

    bool read(msgpack::object_handle& obj);

    template <typename T> bool unpack(T& value) {
        msgpack::object_handle result;
        if (read(result)) {
            msgpack::object obj{result.get()};
            obj.convert(value);
            return true;
        }

        return false;
    }

private:
    msgpack::unpacker unp;
};
} // namespace Engine
