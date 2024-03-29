#include "MsgpackFileWriter.hpp"

using namespace Engine;

MsgpackFileWriter::MsgpackFileWriter(const Path& path) :
    Lz4FileWriter{path}, packer{static_cast<Lz4FileWriter&>(*this)} {
}
