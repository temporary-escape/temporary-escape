#include "msgpack_file_reader.hpp"

using namespace Engine;

static constexpr size_t chunkSize = 4096;

MsgpackFileReader::MsgpackFileReader(const Engine::Path& path) : Lz4FileReader{path} {
}

bool MsgpackFileReader::read(msgpack::object_handle& obj) {
    if (unp.next(obj)) {
        return true;
    }

    while (!eof()) {
        unp.reserve_buffer(chunkSize);

        const auto consumed = Lz4FileReader::read(unp.buffer(), chunkSize);
        unp.buffer_consumed(consumed);

        if (unp.next(obj)) {
            return true;
        }
    }

    return false;
}
