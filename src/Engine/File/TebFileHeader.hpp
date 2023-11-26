#pragma once

#include "../Library.hpp"
#include <msgpack.hpp>

namespace Engine {
enum class TebFileType {
    None,
    Ship,
};

struct ENGINE_API TebFileHeader {
    TebFileHeader();

    std::string version;
    TebFileType type{TebFileType::None};

    MSGPACK_DEFINE_ARRAY(version, type);
};
} // namespace Engine

MSGPACK_ADD_ENUM(Engine::TebFileType)
